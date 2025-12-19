#include <FastLED.h>  // LED制御ライブラリをインクルード

#define NUM_LEDS 256       // LEDの総数（8x32マトリクス）
#define DATA_PIN 6         // LEDデータピンの番号

#define TRIG_PIN 10         // 超音波センサーのトリガーピン
#define ECHO_PIN 9        // 超音波センサーのエコーピン

CRGB leds[NUM_LEDS];       // LEDの配列を定義

String inputString = "";   // シリアル受信用の文字列バッファ
bool receiving = false;    // データ受信中かどうかのフラグ

void setup() {
  Serial.begin(115200);  // シリアル通信を初期化（Processingと通信）
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  // LEDの初期化（WS2812）
  FastLED.clear();       // LEDをすべてオフに
  FastLED.show();        // LEDの状態を反映

  pinMode(TRIG_PIN, OUTPUT);  // トリガーピンを出力に設定
  pinMode(ECHO_PIN, INPUT);   // エコーピンを入力に設定
}

void loop() {
  sendDistance();     // 超音波センサーで距離を測定して送信
  receiveLEDData();   // ProcessingからのLEDデータを受信
  delay(30);          // 少し待機（30ms）
}

void sendDistance() {
  digitalWrite(TRIG_PIN, LOW);       // トリガーをLOWにしてリセット
  delayMicroseconds(2);              // 少し待つ
  digitalWrite(TRIG_PIN, HIGH);      // トリガーをHIGHにして超音波を発射
  delayMicroseconds(10);             // 10マイクロ秒間発射
  digitalWrite(TRIG_PIN, LOW);       // トリガーを再びLOWに

  long duration = pulseIn(ECHO_PIN, HIGH, 20000);  // エコーの戻り時間を取得（最大20ms）
  float distance = duration * 0.034 / 2.0;         // 距離に変換（cm）

  if (distance > 2 && distance < 400) {
    Serial.println(distance);  // 有効な距離ならProcessingに送信
  }
}

void receiveLEDData() {
  while (Serial.available()) {             // シリアルにデータがある間ループ
    char inChar = (char)Serial.read();     // 1文字読み込み
    if (inChar == '\n') {                  // 改行が来たら1行の終わり
      processInput(inputString);           // 入力データを処理
      inputString = "";                    // バッファをリセット
      receiving = false;                   // 受信終了
    } else {
      inputString += inChar;               // バッファに文字を追加
      receiving = true;                    // 受信中フラグを立てる
    }
  }
}

void processInput(String data) {
  if (data.charAt(0) != 'L') return;  // 'L'で始まらないデータは無視

  int values[NUM_LEDS];               // 明るさデータを格納する配列
  int index = 0;
  int lastComma = 0;

  // カンマ区切りの数値をパースして配列に格納
  for (int i = 2; i < data.length(); i++) {
    if (data.charAt(i) == ',' || i == data.length() - 1) {
      String numStr = data.substring(lastComma + 1, (data.charAt(i) == ',') ? i : i + 1);
      values[index] = numStr.toInt();  // 数値に変換して格納
      index++;
      lastComma = i;
      if (index >= NUM_LEDS) break;    // LED数を超えたら終了
    }
  }

  // 各LEDに色と明るさを設定
  for (int i = 0; i < NUM_LEDS; i++) {
    int row = i % 8;  // 縦方向の位置（周波数帯）
    uint8_t hue = map(row, 0, 7, 0, 160);  // 色相を赤〜青にマッピング
    leds[i] = CHSV(hue, 255, values[i]);  // HSVで色と明るさを設定
  }
  FastLED.show();  // LEDに反映
}
