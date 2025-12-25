import com.fazecast.jSerialComm.SerialPort; // シリアル通信（Arduinoとの会話）用
import javax.sound.midi.*;                 // MIDI（楽器の演奏）用
import javax.sound.sampled.*;              // オーディオ（音を聴く）用
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class IntegratedSystem {
    public static void main(String[] args) {
        try {
            // --- 1. MIDIセットアップ（指の役割） ---
            MidiDevice device = null;
            Receiver receiver = null;
            // MacのIACバス（仮想MIDIケーブル）を探すループ
            for (MidiDevice.Info info : MidiSystem.getMidiDeviceInfo()) {
                if (info.getName().contains("IAC")) { 
                    device = MidiSystem.getMidiDevice(info);
                    device.open(); // デバイスを使用可能にする
                    receiver = device.getReceiver(); // 送信口を確保
                    System.out.println("🎹 Connected to MIDI: " + info.getName());
                    break;
                }
            }
            if (receiver == null) { System.out.println("MIDIデバイスが見つかりません！"); return; }

            // --- 2. オーディオキャプチャセットアップ（耳の役割） ---
            // サンプリング周波数 44.1kHz、16bit、モノラルで設定
            AudioFormat format = new AudioFormat(44100, 16, 1, true, false);
            DataLine.Info audioInfo = new DataLine.Info(TargetDataLine.class, format);
            TargetDataLine audioLine = (TargetDataLine) AudioSystem.getLine(audioInfo);
            audioLine.open(format); // 入力ラインを開く
            audioLine.start();      // 音の聴取（BlackHole経由）を開始
            System.out.println("🎧 Audio capture started.");

            // --- 3. シリアルポートセットアップ（神経の役割） ---
            // Arduinoが接続されているポートを指定
            SerialPort port = SerialPort.getCommPort("/dev/cu.usbmodem24EC4A2334102");
            port.setBaudRate(115200); // 通信速度をArduinoと合わせる
            port.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 10, 0); 
            if (!port.openPort()) { System.out.println("シリアルポートを開けませんでした！"); return; }
            System.out.println("✅ Serial port opened.\n");

            // Arduinoからの文字（距離）を読み取るためのリーダー
            BufferedReader reader = new BufferedReader(new InputStreamReader(port.getInputStream()));

            // FFT（高速フーリエ変換）用の計算準備
            int fftSize = 1024;    // 一度に解析する音の細かさ
            int numBands = 16;     // LEDに表示する帯域数
            byte[] audioBuffer = new byte[fftSize * 2]; // 録音データ用
            float[] samples = new float[fftSize];      // 計算用
            int lastNote = -1;                         // 前回鳴らした音を記録

            // --- 4. メインループ（思考の回転） ---
            while (true) {
                try {
                    // --- (A) 音波解析とLEDデータ送信 ---
                    if (audioLine.available() >= audioBuffer.length) {
                        audioLine.read(audioBuffer, 0, audioBuffer.length);

                        // バイトデータを数値（波形）に変換
                        for (int i = 0; i < fftSize; i++) {
                            int low = audioBuffer[2 * i] & 0xFF;
                            int high = audioBuffer[2 * i + 1];
                            samples[i] = (short)((high << 8) | low) / 32768.0f;
                        }

                        // 数学の力で音を高さごとに分解（FFT実行）
                        double[] real = new double[fftSize];
                        double[] imag = new double[fftSize];
                        for (int i = 0; i < fftSize; i++) { real[i] = samples[i]; imag[i] = 0.0; }
                        FFT.fft(real, imag);

                        // 16個のLED用データにまとめる
                        byte[] sendData = new byte[numBands];
                        int bandSize = (fftSize / 2) / numBands;
                        for (int i = 0; i < numBands; i++) {
                            double sum = 0;
                            for (int j = 0; j < bandSize; j++) {
                                // 音のエネルギー（強さ）を計算
                                sum += Math.sqrt(real[i * bandSize + j] * real[i * bandSize + j] +
                                                imag[i * bandSize + j] * imag[i * bandSize + j]);
                            }
                            double scaled = (sum / bandSize) * 60.0; // 見えやすく感度を調整
                            sendData[i] = (byte) Math.min(255, (int)scaled);
                        }

                        // ヘッダー(0xFF)をつけてArduinoへ一気に送信
                        port.writeBytes(new byte[]{(byte)0xFF}, 1);
                        port.writeBytes(sendData, sendData.length);
                    }

                    // --- (B) 手の距離に応じた演奏（MIDI送信） ---
                    if (port.bytesAvailable() > 0) {
                        String line = reader.readLine();
                        if (line != null && !line.trim().isEmpty()) {
                            float distance = Float.parseFloat(line.trim());

                            // 鳴らす音階（ドレミ...）の定義
                            int[] midiNotes = {57, 59, 60, 62, 64, 65, 67, 69};
                            // 距離(5cm~40cm)を音階のインデックス(0~7)に変換
                            int index = (int) map(distance, 5, 40, 0, midiNotes.length - 1);
                            index = Math.max(0, Math.min(index, midiNotes.length - 1));
                            int note = midiNotes[index];

                            // 手の位置が変わった時だけ新しい音を鳴らす
                            if (note != lastNote) {
                                if (lastNote != -1) receiver.send(new ShortMessage(ShortMessage.NOTE_OFF, 0, lastNote, 0), -1);
                                receiver.send(new ShortMessage(ShortMessage.NOTE_ON, 0, note, 100), -1);
                                lastNote = note;
                            }
                        }
                    }

                    Thread.sleep(10); // PCのCPU負荷を抑えるための休憩

                } catch (Exception e) {
                    // 通信エラー等が発生してもプログラムを止めずにループを維持
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // 数値の範囲を変換する便利な計算用関数（Arduinoのmap関数と同じ役割）
    public static float map(float val, float inMin, float inMax, float outMin, float outMax) {
        return (val - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    }
}
