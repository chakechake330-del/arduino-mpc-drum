import ddf.minim.*;
import ddf.minim.analysis.*;
import processing.serial.*;
import themidibus.*;

Minim minim;
AudioInput in;
FFT fft;
Serial myPort;
MidiBus midi;

int cols = 32;  // 横方向（時間）
int rows = 8;   // 縦方向（周波数）
int[][] spectrumHistory = new int[cols][rows];

String distanceBuffer = "";

void setup() {
  size(600, 400);
  minim = new Minim(this);
  in = minim.getLineIn(Minim.MONO, 512);
  fft = new FFT(in.bufferSize(), in.sampleRate());

  printArray(Serial.list());
  String portName = Serial.list()[5];  // 適切なポート番号に変更
  myPort = new Serial(this, portName, 115200);

  MidiBus.list();  // 利用可能なMIDIポートを確認
  midi = new MidiBus(this, -1, "IAC Driver Bus 1");  // 出力先を指定
}

void draw() {
  background(0);
  updateSpectrum();
  sendToLED();
  drawVisualizer();
}

void updateSpectrum() {
  for (int x = cols - 1; x > 0; x--) {
    for (int y = 0; y < rows; y++) {
      spectrumHistory[x][y] = spectrumHistory[x - 1][y];
    }
  }

  fft.forward(in.mix);
  for (int y = 0; y < rows; y++) {
    float level = fft.getBand(y);
    int brightness = int(constrain(level * 10, 0, 255));
    spectrumHistory[0][y] = brightness;
  }
}

void sendToLED() {
  String data = "L:";
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {
      data += spectrumHistory[i][j];
      if (!(i == cols - 1 && j == rows - 1)) data += ",";
    }
  }
  myPort.write(data + "\n");
}

void drawVisualizer() {
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {
      fill(spectrumHistory[i][j], 255, 255);
      rect(i * (width / cols), height - j * (height / rows), width / cols, -(height / rows));
    }
  }
}

void serialEvent(Serial p) {
  while (p.available() > 0) {
    char inChar = p.readChar();
    if (inChar == '\n') {
      processDistance(distanceBuffer.trim());
      distanceBuffer = "";
    } else {
      distanceBuffer += inChar;
    }
  }
}

void processDistance(String raw) {
  try {
    float distance = Float.parseFloat(raw);
    println("📏 距離: " + distance + " cm");

    int note = int(map(distance, 5, 100, 80, 40));
    note = constrain(note, 40, 80);
    sendMIDINote(note);
  } catch (Exception e) {
    println("⚠️ 距離データの解析に失敗: " + raw);
  }
}

void sendMIDINote(int note) {
  midi.sendNoteOn(0, note, 100);
  delay(100);
  midi.sendNoteOff(0, note, 100);
}
