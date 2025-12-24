#include <Adafruit_NeoPixel.h>

const int numBands = 16;
const int matrixWidth = 32;
const int matrixHeight = 8;
const int numPixels = matrixWidth * matrixHeight;
const int dataPin = 6;
const int trigPin = 9;
const int echoPin = 10;

Adafruit_NeoPixel strip(numPixels, dataPin, NEO_GRB + NEO_KHZ800);
byte bands[numBands];
int peaks[numBands];

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(100);
  strip.show();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  for (int i = 0; i < numBands; i++) peaks[i] = 0;
}

void loop() {
  // --- 1. 超音波センサー測定 ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // タイムアウト設定でフリーズを防止
  long duration = pulseIn(echoPin, HIGH, 25000); 
  float distance = duration * 0.034 / 2.0;
  
  // 距離を送信（MIDI制御用）
  if (distance > 0) {
    Serial.println(distance);
  }

  // --- 2. JavaからのFFTデータ受信（ヘッダー同期型） ---
  // 0xFF (1byte) + 16bands (16bytes) = 17bytes 以上の時に処理
  while (Serial.available() > numBands) {
    if (Serial.read() == 0xFF) { // ヘッダー発見
      Serial.readBytes(bands, numBands); // まとめて読み込む
      
      // LED表示更新
      updateDisplay();
      break; // 最新のデータのみを処理して抜ける
    }
  }

  // 100msだとMIDIの反応が遅いため、30ms程度に短縮するのがおすすめ
  delay(30); 
}

void updateDisplay() {
  strip.clear();
  for (int x = 0; x < numBands; x++) {
    int value = bands[x];
    int height = map(value, 0, 255, 0, matrixHeight);

    // ピーク更新
    if (height > peaks[x]) peaks[x] = height;
    else if (peaks[x] > 0) peaks[x]--;

    // 縦棒の描画
    int hue = map(x, 0, numBands - 1, 0, 65535);
    int baseX = x * 2;
    for (int y = 0; y < height; y++) {
      int brightness = map(y, 0, matrixHeight - 1, 100, 255);
      uint32_t color = strip.ColorHSV(hue, 255, brightness);
      
      for (int dx = 0; dx < 2; dx++) {
        strip.setPixelColor(getPixelIndex(baseX + dx, matrixHeight - 1 - y), color);
      }
    }

    // ピーク点（赤）
    if (peaks[x] > 0) {
      int peakY = matrixHeight - 1 - peaks[x];
      for (int dx = 0; dx < 2; dx++) {
        strip.setPixelColor(getPixelIndex(baseX + dx, peakY), strip.Color(255, 0, 0));
      }
    }
  }
  strip.show();
}

int getPixelIndex(int x, int y) {
  if (x < 0 || x >= matrixWidth || y < 0 || y >= matrixHeight) return 0;
  if (y % 2 == 0) return y * matrixWidth + x;
  else return y * matrixWidth + (matrixWidth - 1 - x);
}
