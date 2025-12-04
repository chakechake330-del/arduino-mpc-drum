#include <Adafruit_NeoPixel.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include <FFT.h>
#include <ErriezTM1637.h>

// === ピン定義 ===
#define LED_PIN     6
#define AUDIO_PIN   A0
#define DF_TX       10
#define DF_RX       11
#define KP_CLK      3
#define KP_DIO      4

// === LEDマトリクス設定 ===
#define LED_WIDTH   32
#define LED_HEIGHT  8
#define NUM_LEDS    (LED_WIDTH * LED_HEIGHT)
Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// === DFPlayer設定 ===
SoftwareSerial dfSerial(DF_RX, DF_TX);
DFRobotDFPlayerMini dfplayer;

// === キーパッド設定 ===
ErriezTM1637 keypad(KP_CLK, KP_DIO);

// === FFT設定 ===
#define SAMPLES         128
#define SAMPLING_FREQ   4000
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[LED_WIDTH];

void setup() {
  Serial.begin(9600);

  // LED初期化
  leds.begin();
  leds.setBrightness(40);
  leds.show();

  // DFPlayer初期化
  dfSerial.begin(9600);
  if (!dfplayer.begin(dfSerial)) {
    Serial.println("DFPlayer Mini not found!");
    while (1);
  }
  dfplayer.volume(20);
  Serial.println("DFPlayer ready");

  // キーパッド初期化
  keypad.begin();
  keypad.setBrightness(7);

  // FFT初期化
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));
}

void loop() {
  // === キーパッド読み取り ===
  int key = keypad.getKey();
  if (key != -1) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    dfplayer.play(key + 1);  // キー0→0001.mp3
  }

  // === サンプリング ===
  for (int i = 0; i < SAMPLES; i++) {
    microseconds = micros();
    int val = analogRead(AUDIO_PIN);
    fft_input[i] = val;
    while (micros() - microseconds < sampling_period_us);
  }

  // === FFT処理 ===
  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();

  // === LED表示 ===
  leds.clear();
  for (int x = 0; x < LED_WIDTH; x++) {
    int bin = map(x, 0, LED_WIDTH - 1, 2, SAMPLES / 2 - 1);
    int level = map(fft_log_out[bin], 0, 100, 0, LED_HEIGHT);
    level = constrain(level, 0, LED_HEIGHT);

    for (int y = 0; y < level; y++) {
      int idx = getLedIndex(x, y);
      leds.setPixelColor(idx, leds.Color(0, 150, 255));  // 青系
    }
  }
  leds.show();

  delay(30);
}

// === LEDマトリクスのインデックス計算（ジグザグ配線） ===
int getLedIndex(int x, int y) {
  if (x % 2 == 0) {
    return x * LED_HEIGHT + y;
  } else {
    return x * LED_HEIGHT + (LED_HEIGHT - 1 - y);
  }
}
