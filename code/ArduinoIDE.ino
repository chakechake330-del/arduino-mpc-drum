#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUM_PIXELS 512  
#define BRIGHTNESS 80

Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

String input = "";
int levels[16];

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      parseData(input);
      input = "";
      displayBars();
    } else {
      input += c;
    }
  }
}

void parseData(String data) {
  int index = 0;
  int lastIndex = 0;
  for (int i = 0; i < 16; i++) {
    index = data.indexOf(',', lastIndex);
    if (index == -1 && i < 15) return;
    String val = (i < 15) ? data.substring(lastIndex, index) : data.substring(lastIndex);
    levels[i] = constrain(val.toInt() / 4, 0, 32); 
    lastIndex = index + 1;
  }
}

void displayBars() {
  strip.clear();

  for (int x = 0; x < 16; x++) {
    int height = levels[x];
    for (int y = 0; y < height; y++) {
      int pixelIndex = getPixelIndex(x, y);
      uint32_t color = getColorForLevel(y);
      strip.setPixelColor(pixelIndex, color);
    }
  }

  strip.show();
}

int getPixelIndex(int x, int y) {
  if (x % 2 == 0) {
    return x * 32 + y;
  } else {
    return x * 32 + (31 - y);
  }
}

uint32_t getColorForLevel(int level) {
  int r = map(level, 0, 31, 0, 255);
  int g = 255 - abs(16 - level) * 8;
  int b = 255 - r;
  return strip.Color(r, g, b);
}
