import ddf.minim.*;
import ddf.minim.analysis.*;
import processing.serial.*;

Minim minim;
AudioInput in;
FFT fft;

Serial myPort;

int bands = 32;
int[] spectrumData = new int[bands];

void setup() {
  size(512, 200);
  
  printArray(Serial.list());
  myPort = new Serial(this, Serial.list()[5], 115200);  // ← ポート番号は要確認！

  minim = new Minim(this);
  in = minim.getLineIn(Minim.MONO, 512);
  fft = new FFT(in.bufferSize(), in.sampleRate());
}

void draw() {
  background(0);
  
  fft.forward(in.mix);

  for (int i = 0; i < bands; i++) {
    int start = i * (fft.specSize() / bands);
    int end = (i + 1) * (fft.specSize() / bands);
    float sum = 0;
    for (int j = start; j < end; j++) {
      sum += fft.getBand(j);
    }
    float avg = sum / (end - start);
    spectrumData[i] = int(map(avg, 0, 50, 0, 255));
    spectrumData[i] = constrain(spectrumData[i], 0, 255);

    // 可視化
    float barWidth = width / float(bands);
    fill(255);
    rect(i * barWidth, height, barWidth - 2, -spectrumData[i]);
  }

  // Arduinoに送信
  String out = "L";
  for (int i = 0; i < bands; i++) {
    out += "," + spectrumData[i];
  }
  out += "\n";
  myPort.write(out);
}
