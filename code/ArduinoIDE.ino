#include <Adafruit_NeoPixel.h> // LED制御用のライブラリを読み込み

// --- 設定値の定義 ---
const int numBands = 16;       // 音楽を16個の周波数帯（低音〜高音）に分けて表示
const int matrixWidth = 32;    // LEDパネルの横幅
const int matrixHeight = 8;    // LEDパネルの高さ
const int numPixels = matrixWidth * matrixHeight; // LEDの総数（32x8=256個）
const int dataPin = 6;         // LEDを制御する信号を送るピン番号
const int trigPin = 10;         // 超音波センサーの送信ピン
const int echoPin = 9;        // 超音波センサーの受信ピン

// LEDオブジェクトの作成（信号の種類や色の並び順を設定）
Adafruit_NeoPixel strip(numPixels, dataPin, NEO_GRB + NEO_KHZ800);

byte bands[numBands];          // Javaから届く16個の音量データを入れる箱
int peaks[numBands];           // 棒グラフの一番高いところ（ピーク）を保持する記録用

void setup() {
  Serial.begin(115200);       // PCと高速で通信するための設定
  strip.begin();              // LEDを光らせる準備を開始
  strip.setBrightness(100);    // LEDの明るさを調整（最大255）
  strip.show();               // 一度すべてのLEDをオフにする

  pinMode(trigPin, OUTPUT);    // 超音波を出すためにトリガーピンを出力に設定
  pinMode(echoPin, INPUT);     // 跳ね返った音を聴くためにエコーピンを入力に設定

  for (int i = 0; i < numBands; i++) peaks[i] = 0; // ピーク記録をリセット
}

void loop() {
  // --- 1. 超音波センサーで手の距離を測る ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); // 超音波を発射！
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 音が跳ね返ってくるまでの時間を測る（タイムアウト設定でフリーズを防止）
  long duration = pulseIn(echoPin, HIGH, 25000); 
  float distance = duration * 0.034 / 2.0;       // 時間から距離(cm)に計算
  
  // 距離データをPC（Java）へ送信（MIDI制御用）
  if (distance > 0) {
    Serial.println(distance);
  }

  // --- 2. Javaからの音波解析データを受信 ---
  // データの目印(0xFF) + 16個のデータ = 計17バイト届いているか確認
  while (Serial.available() > numBands) {
    if (Serial.read() == 0xFF) {       // 「ここからデータが始まるよ」という合図を見つけたら
      Serial.readBytes(bands, numBands); // 16個のデータをまとめて読み込む
      updateDisplay();                 // LEDの表示を更新する
      break;                           // 最新のデータのみを処理して次へ
    }
  }

  delay(30); // 1秒間に約33回ループするように調整（リアルタイム性を確保）
}

// --- LEDを光らせるメイン処理 ---
void updateDisplay() {
  strip.clear(); // まず画面を真っ暗にする
  for (int x = 0; x < numBands; x++) {
    int value = bands[x]; // Javaから届いた音の強さ（0-255）
    // 音の強さをLEDの高さ（0-8マス）に変換
    int height = map(value, 0, 255, 0, matrixHeight);

    // ピーク（頂点）を滑らかに落とすアニメーション処理
    if (height > peaks[x]) peaks[x] = height; // 今の高さが最高なら更新
    else if (peaks[x] > 0) peaks[x]--;        // 低ければ少しずつ落とす

    // 縦棒の色と明るさを設定
    int hue = map(x, 0, numBands - 1, 0, 65535); // 音の高さごとに色を変える（虹色）
    int baseX = x * 2; // 1つの帯域を2ドット幅で表示
    for (int y = 0; y < height; y++) {
      int brightness = map(y, 0, matrixHeight - 1, 100, 255); // 上に行くほど明るく
      uint32_t color = strip.ColorHSV(hue, 255, brightness);
      
      for (int dx = 0; dx < 2; dx++) {
        // LEDパネル特有の座標計算をしてセット
        strip.setPixelColor(getPixelIndex(baseX + dx, matrixHeight - 1 - y), color);
      }
    }

    // ピーク点（赤色）を描画
    if (peaks[x] > 0) {
      int peakY = matrixHeight - 1 - peaks[x];
      for (int dx = 0; dx < 2; dx++) {
        strip.setPixelColor(getPixelIndex(baseX + dx, peakY), strip.Color(255, 0, 0));
      }
    }
  }
  strip.show(); // 設定した内容を一気にLEDに反映！
}

// LEDパネルのジグザグ配線に対応した座標変換関数
int getPixelIndex(int x, int y) {
  if (x < 0 || x >= matrixWidth || y < 0 || y >= matrixHeight) return 0;
  // 奇数行目が反転しているパネルの特性に合わせた計算式
  if (y % 2 == 0) return y * matrixWidth + x;
  else return y * matrixWidth + (matrixWidth - 1 - x);
}
