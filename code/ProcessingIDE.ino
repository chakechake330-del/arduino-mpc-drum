// 必要なライブラリをインポート
import ddf.minim.*;               // 音声入力用ライブラリ
import ddf.minim.analysis.*;      // FFT解析用ライブラリ
import processing.serial.*;       // シリアル通信用ライブラリ
import themidibus.*;              // MIDI通信用ライブラリ

// ライブラリ用のオブジェクトを宣言
Minim minim;
AudioInput in;
FFT fft;
Serial myPort;
MidiBus midi;

// スペクトラム表示用の行列サイズ
int cols = 32;  // 横方向（時間）
int rows = 8;   // 縦方向（周波数）
int[][] spectrumHistory = new int[cols][rows];  // スペクトラム履歴を保存する配列

// 距離センサーのデータ処理用変数
String distanceBuffer = "";       // シリアルからの文字列バッファ
float lastDistance = -1;          // 前回の距離
float distanceThreshold = 3.0;    // 距離変化のしきい値（cm）

void setup() {
  size(600, 400);  // ウィンドウサイズ設定
  minim = new Minim(this);  // Minimの初期化
  in = minim.getLineIn(Minim.MONO, 512);  // モノラル音声入力を取得
  fft = new FFT(in.bufferSize(), in.sampleRate());  // FFT解析の準備

  printArray(Serial.list());  // 利用可能なシリアルポートを表示
  String portName = Serial.list()[5];  // 使用するポート名（環境に応じて変更）
  myPort = new Serial(this, portName, 115200);  // シリアルポートを初期化

  MidiBus.list();  // 利用可能なMIDIポートを表示
  midi = new MidiBus(this, -1, "IAC Bus 1");  // MIDIバスの初期化（出力先を指定）
}

void draw() {
  background(0);       // 背景を黒で塗りつぶす
  updateSpectrum();    // スペクトラムデータを更新
  sendToLED();         // LEDにデータを送信
  drawVisualizer();    // ビジュアライザーを描画
}

void updateSpectrum() {
  // スペクトラム履歴を1列右にずらす
  for (int x = cols - 1; x > 0; x--) {
    for (int y = 0; y < rows; y++) {
      spectrumHistory[x][y] = spectrumHistory[x - 1][y];
    }
  }

  fft.forward(in.mix);  // 現在の音声データにFFTを適用
  for (int y = 0; y < rows; y++) {
    float level = fft.getBand(y);  // 各周波数帯のレベルを取得
    int brightness = int(constrain(level * 10, 0, 255));  // 明るさに変換（0〜255に制限）
    spectrumHistory[0][y] = brightness;  // 最新データを左端に追加
  }
}

void sendToLED() {
  String data = "L:";  // LED用データの先頭識別子
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {
      data += spectrumHistory[i][j];  // 明るさデータを文字列に追加
      if (!(i == cols - 1 && j == rows - 1)) data += ",";  // 最後以外はカンマ区切り
    }
  }
  myPort.write(data + "\n");  // シリアルポートに送信
}

void drawVisualizer() {
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {
      fill(spectrumHistory[i][j], 255, 255);  // 色を設定（明るさに応じた色）
      rect(i * (width / cols), height - j * (height / rows), width / cols, -(height / rows));  // 矩形を描画
    }
  }
}

void serialEvent(Serial p) {
  while (p.available() > 0) {
    char inChar = p.readChar();  // 1文字読み込み
    if (inChar == '\n') {
      processDistance(distanceBuffer.trim());  // 改行が来たら距離データを処理
      distanceBuffer = "";  // バッファをリセット
    } else {
      distanceBuffer += inChar;  // バッファに文字を追加
    }
  }
}

void processDistance(String raw) {
  try {
    float distance = Float.parseFloat(raw);  // 文字列をfloatに変換
    println(" 距離: " + distance + " cm");

    // 前回と比べて変化が小さいなら無視
    if (lastDistance >= 0 && abs(distance - lastDistance) < distanceThreshold) {
      return;
    }

    lastDistance = distance;  // 距離を更新

    float rawNote = map(distance, 5, 100, 100, 20);  // 距離をMIDIノートにマッピング
    int note = round(constrain(rawNote, 20, 100));  // ノート番号を制限して丸める
    sendMIDINote(note);  // MIDIノートを送信
  } catch (Exception e) {
    println(" 距離データの解析に失敗: " + raw);  // パース失敗時のエラーメッセージ
  }
}

void sendMIDINote(int note) {
  println(" 送信中のMIDIノート: " + note);
  midi.sendNoteOn(0, note, 100);  // ノートオン（チャンネル0、ベロシティ100）
  delay(100);                     // 少し待つ
  midi.sendNoteOff(0, note, 100);  // ノートオフ
}
