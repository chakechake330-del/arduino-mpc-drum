import processing.sound.*;
import processing.serial.*;

FFT fft;
AudioIn in;
Serial myPort;

int bands = 8;
int[] levels = new int[bands];

void setup() {
  size(600, 400);
  printArray(Serial.list());
  String portName = Serial.list()[5];  // 必要に応じてポート番号を変更！
  myPort = new Serial(this, portName, 115200);

  fft = new FFT(this, bands);
  in = new AudioIn(this, 0);
  in.start();
  fft.input(in);
}

void draw() {
  background(0);
  fft.analyze();

  String data = "L:";
  for (int i = 0; i < bands; i++) {
    float level = fft.spectrum[i];
    int brightness = int(constrain(level * 5000, 0, 255));
    levels[i] = brightness;
    data += brightness;
    if (i < bands - 1) data += ",";
  }
    myPort.write(data + "\n");

  // デバッグ用ビジュアライザ
  for (int i = 0; i < bands; i++) {
    fill(levels[i], 255, 255);
    rect(i * (width / bands), height, width / bands, -levels[i]);
  }
}
