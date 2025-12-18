#include <FastLED.h>

#define NUM_LEDS 256
#define DATA_PIN 6

#define TRIG_PIN 9
#define ECHO_PIN 10

CRGB leds[NUM_LEDS];

String inputString = "";
bool receiving = false;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  sendDistance();     // 超音波センサーの値を送信
  receiveLEDData();   // ProcessingからのLEDデータを受信
  delay(30);
}

void sendDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  float distance = duration * 0.034 / 2.0;

  if (distance > 2 && distance < 400) {
    Serial.println(distance);  // Processingに送信
  }
}

void receiveLEDData() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      processInput(inputString);
      inputString = "";
      receiving = false;
    } else {
      inputString += inChar;
      receiving = true;
    }
  }
}

void processInput(String data) {
  if (data.charAt(0) != 'L') return;

  int values[NUM_LEDS];
  int index = 0;
  int lastComma = 0;

  for (int i = 2; i < data.length(); i++) {
    if (data.charAt(i) == ',' || i == data.length() - 1) {
      String numStr = data.substring(lastComma + 1, (data.charAt(i) == ',') ? i : i + 1);
      values[index] = numStr.toInt();
      index++;
      lastComma = i;
      if (index >= NUM_LEDS) break;
    }
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    int row = i % 8;  // 縦方向（周波数帯）
    uint8_t hue = map(row, 0, 7, 0, 160);  // 赤〜青のグラデーション
    leds[i] = CHSV(hue, 255, values[i]);
  }
  FastLED.show();
}
