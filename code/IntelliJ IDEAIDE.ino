import com.fazecast.jSerialComm.SerialPort;
import javax.sound.midi.*;
import javax.sound.sampled.*;
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class IntegratedSystem {
    public static void main(String[] args) {
        try {
            // --- 1. MIDIセットアップ ---
            MidiDevice device = null;
            Receiver receiver = null;
            for (MidiDevice.Info info : MidiSystem.getMidiDeviceInfo()) {
                if (info.getName().contains("IAC")) { // MacのIACバスなどを想定
                    device = MidiSystem.getMidiDevice(info);
                    device.open();
                    receiver = device.getReceiver();
                    System.out.println("🎹 Connected to MIDI: " + info.getName());
                    break;
                }
            }
            if (receiver == null) { System.out.println("MIDI device not found!"); return; }

            // --- 2. オーディオキャプチャ（マイク入力）セットアップ ---
            AudioFormat format = new AudioFormat(44100, 16, 1, true, false);
            DataLine.Info audioInfo = new DataLine.Info(TargetDataLine.class, format);
            TargetDataLine audioLine = (TargetDataLine) AudioSystem.getLine(audioInfo);
            audioLine.open(format);
            audioLine.start();
            System.out.println("🎧 Audio capture started.");

            // --- 3. シリアルポートセットアップ ---
            SerialPort port = SerialPort.getCommPort("/dev/cu.usbmodem24EC4A2334102");
            port.setBaudRate(115200);
            port.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 10, 0); // タイムアウトを短く
            if (!port.openPort()) { System.out.println("Failed to open serial port!"); return; }
            System.out.println("✅ Serial port opened.\n");

            BufferedReader reader = new BufferedReader(new InputStreamReader(port.getInputStream()));

            // FFT用変数
            int fftSize = 1024;
            int numBands = 16;
            byte[] audioBuffer = new byte[fftSize * 2];
            float[] samples = new float[fftSize];
            int lastNote = -1;

            // --- 4. メインループ ---
            while (true) {
                try {
                    // --- (A) FFT解析とデータ送信 ---
                    if (audioLine.available() >= audioBuffer.length) {
                        audioLine.read(audioBuffer, 0, audioBuffer.length);

                        // byte -> float 変換
                        for (int i = 0; i < fftSize; i++) {
                            int low = audioBuffer[2 * i] & 0xFF;
                            int high = audioBuffer[2 * i + 1];
                            samples[i] = (short)((high << 8) | low) / 32768.0f;
                        }

                        // FFT実行
                        double[] real = new double[fftSize];
                        double[] imag = new double[fftSize];
                        for (int i = 0; i < fftSize; i++) { real[i] = samples[i]; imag[i] = 0.0; }
                        FFT.fft(real, imag);

                        // バンド分割
                        byte[] sendData = new byte[numBands];
                        int bandSize = (fftSize / 2) / numBands;
                        for (int i = 0; i < numBands; i++) {
                            double sum = 0;
                            for (int j = 0; j < bandSize; j++) {
                                sum += Math.sqrt(real[i * bandSize + j] * real[i * bandSize + j] +
                                        imag[i * bandSize + j] * imag[i * bandSize + j]);
                            }
                            double scaled = (sum / bandSize) * 60.0; // 感度調整係数
                            sendData[i] = (byte) Math.min(255, (int)scaled);
                        }

                        // ヘッダー(0xFF) + データの送信
                        port.writeBytes(new byte[]{(byte)0xFF}, 1);
                        port.writeBytes(sendData, sendData.length);
                    }

                    // --- (B) 距離受信とMIDI送信 ---
                    if (port.bytesAvailable() > 0) {
                        String line = reader.readLine();
                        if (line != null && !line.trim().isEmpty()) {
                            float distance = Float.parseFloat(line.trim());

                            int[] midiNotes = {57, 59, 60, 62, 64, 65, 67, 69};
                            int index = (int) map(distance, 5, 40, 0, midiNotes.length - 1);
                            index = Math.max(0, Math.min(index, midiNotes.length - 1));
                            int note = midiNotes[index];

                            if (note != lastNote) {
                                if (lastNote != -1) receiver.send(new ShortMessage(ShortMessage.NOTE_OFF, 0, lastNote, 0), -1);
                                receiver.send(new ShortMessage(ShortMessage.NOTE_ON, 0, note, 100), -1);
                                lastNote = note;
                            }
                        }
                    }

                    Thread.sleep(10); // ループの過負荷防止

                } catch (Exception e) {
                    // 通信途切等
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static float map(float val, float inMin, float inMax, float outMin, float outMax) {
        return (val - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    }
}
