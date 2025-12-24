# 卒業制作2025
## 1.  概要 
このプロジェクトは、空中に手をかざすだけで演奏ができ、その音がリアルタイムで光に変わる**エアー楽器**です。  
超音波センサーが「手との距離」を瞬時に読み取り、それを「MIDI（ミディ）信号」に変換します。  
このMIDI信号が「今、この音を鳴らして」とパソコン内の音楽ソフト（Logic Pro）に伝えることで、  
ドラムやシンセサイザーの音を自由自在に鳴らすことができます。  
さらに、鳴った音はすぐに分析され、音の高さやリズムに合わせてLEDが光ります。

### ※MIDI（ミディ）信号とは？  
電子楽器やコンピューターの間で演奏情報をやり取りするための**デジタル言語**です。  
「どの音を（音程）」「どれくらいの強さで（音量）」といった指示がデータ化されており、  
本プロジェクトでは手の距離をこのMIDI信号に翻訳して音を鳴らしています

## 2.  デモ動画
[![Demo]()
### *主な機能*

- **超音波モジュール**  
  センサーの前に手をかざすと、距離に応じたMIDIノートを生成し、Logic Proでドラム音や効果音を再生します。
　
- **Logic Pro**  
  ArduinoからのMIDI信号をLogic Proで受信し、ソフトウェア音源を鳴らす

- **FFT解析（PC側）**  
  Logic Proの音声出力をPCで取得し、リアルタイムFFT解析を実行する

- **WS2812B LEDマトリクス**  
  解析結果に基づき、8×32のLEDマトリクスにスペクトラムアナライザを表示します。

- **Arduino Uno R4 WiFi**  
  超音波センサーの読み取り、MIDIノートの送信、LED制御を1台で実行します。

### *システムデータフロー*

1.  Arduino Uno R4 WiFi が超音波センサーで距離を読み取り、MIDI信号に変換してUSB経由で送信
2.  Logic Pro が IAC Driver 経由でMIDI信号を受信し、ソフト音源を再生
3.  Logic Proの音声出力 を PC内部の「見えないケーブル（BlackHole）」を通してProcessingへ送ります
4.  Processing がFFT解析を行い、その情報を再びArduinoへ送り返してLEDを光らせます

## 3.  仕様書

### *配線図*
> ※配線図にはブレッドボード用の電源モジュールは含まれていませんが、このプロジェクトでは電源モジュールの使用することをおすすめします。  
> ※この配線図はFritzingの都合により、以下の部品を代用品で表現しています
> - LEDマトリクス：実際には8×32のWS2812Bマトリクス（データピン：D6）を使用

![配線図](images_and_videos/Arduino_drum_配線図.png)

### *回路図*

![回路図](images_and_videos/Arduino_drum_回路図.png)

### *使用モジュールとピン*


| モジュール名                 | 用途                                | 使用ピン（Arduino Uno R4 WiFi）  |
|---------------------------|-------------------------------------|------------------------------|
| 超音波距離センサー（HC-SR04など） | 手の距離を測定し、MIDIノートを決定        |      TRIG: D10 / ECHO: D9    |
| WS2812B LEDマトリクス（8×32）  | スペクトラムアナライザ表示               | データピン: D6                  |
| Arduino Uno R4 WiFi         | 全体制御（MIDI送信・LED制御）         | -                             |
| PC（Logic Pro + Processing）     | 音声再生・FFT解析・LED制御         | USBシリアル通信                  |

### 使用ツール・環境

- **Arduino IDE**（統合開発環境 / マイコン用コードの開発・書き込み）
- **Processing バージョン：3.5.4（推奨）**（ビジュアルプログラミング環境 / 音声解析・シリアル通信・LED制御）
- **Logic Pro**（DAW / MIDI受信と音声出力）
- **BlackHole**（仮想オーディオルーティング / ProcessingでLogic Proの音を取得）
- **Audio MIDI設定（IACドライバ）**（Mac標準 / 仮想MIDIポートの作成・接続）

### Arduino使用ライブラリ

- **FastLED**(256個のLEDを高速かつ個別に制御するために使用しています)

### Processing使用ライブラリ

- **Minim**(複雑な音波を周波数ごとに分解（FFT解析）するために使用しています。)
- **TheMidiBus**(Processingを仮想的なMIDIキーボードとして動作させるために使用しています)
- **processing.serial**(Arduinoとのシリアル通信用ライブラリ)

## 4.  システムブロック図
![ブロック図](images_and_videos/mermaid-diagram-2025-12-22-172352.png)

## 5.  フローチャート

###  *[MIDI楽器コントローラ(Arduino側)]*

```mermaid
flowchart TD
    A[起動] --> B["初期化<br/>(センサー準備 / <b>FastLED</b>設定)"]
    B --> C[PCとの通信開始]
    C --> D[メインループ開始]

    D --> E["距離計測<br/>(超音波センサー)"]
    E --> F["距離の計算<br/>(音速からcmを算出)"]
    F --> G["PCへ距離を送信<br/>(シリアル通信)"]

    G --> H["PCから映像データを受信<br/>(シリアル通信)"]
    H --> I["データの解析<br/>(輝度情報への変換)"]
    I --> J["LEDマトリクス点灯<br/><b>【FastLEDライブラリ】</b>"]
    J --> D
```

###  *[音響解析・映像生成(Processing側)]*

```mermaid
flowchart TD
    A[起動] --> B["音声解析の準備<br/><b>【Minimライブラリ】</b>"]
    B --> C["通信の準備<br/>(シリアル / <b>TheMidiBus</b>)"]
    C --> D[メインループ開始]

    D --> E["Logic Proの音を取得<br/><b>(BlackHole経由)</b>"]
    E --> F["周波数解析<br/><b>【FFT実行】</b>"]
    F --> G["音の変化(スペクトラム)を記録"]
    G --> H["LED用の映像データを生成"]
    H --> I["コントローラ(arduino)へ映像送信<br/>(シリアル通信)"]

    D --> J["超音波センサーから距離を受信"]
    J --> K["前回の距離と比較"]
    K --> L{一定以上の動き？}
    L -- Yes --> M["MIDI信号を送信<br/><b>【TheMidiBusライブラリ】</b>"]
    L -- No --> N[処理スキップ]

    M --> D
    N --> D
```


## 6.  使用ツールの詳細

### 🔹**Processing** 
Javaベースのビジュアルプログラミング環境。   
このプロジェクトでは、Logic Proの音声をリアルタイムにFFT解析し、LEDマトリクスにスペクトラム表示する役割を担います。   
また、Arduinoとのシリアル通信を通じて、LED制御データを送信します。  

### 🔹**BlackHole**  
BlackHoleは、macOS用の仮想オーディオドライバです。    
通常、アプリの音声はスピーカーに直接送られますが、BlackHoleを使うことで、その音声を別のアプリ（この場合はProcessing）に受け渡すことができます。
このプロジェクトでは、Logic Proで鳴った音をProcessingに届ける“音の受け渡し役”としてBlackHoleを使用しています。

### 🔹**Audio MIDI設定（IACドライバ）**（macOS標準機能）  
macOS標準のMIDIルーティングツール。   
ArduinoからのMIDIノートをLogic Proに送信するための仮想MIDIポートを作成します。


## 7.  工夫ポイント

### ◎*ArduinoとProcessingのリアルタイムやりとり*  
ArduinoとProcessingの間で、
Arduino → Processing：距離センサーのデータを送信  
Processing → Arduino：LEDの表示データを送信  
このように双方向でやり取りすることで、動きや音にすぐ反応する仕組みを実現しています。

### ◎*リアルタイムFFT解析*  
Logic Proで再生された音をBlackHoleを使ってProcessingに取り込み、 その場で周波数解析（FFT）を行います。  
音の変化に応じて、LEDの光り方をすぐに変化させています。

### ◎*距離変化のしきい値処理*   
手の位置がほとんど動いていないときは、MIDIノートを送らないようにしています。  
これにより、意図しない連打やノイズを防ぎ、自然な演奏感を保っています。


## 8.  参考サイト

- [基本プロジェクト 超音波](https://docs.sunfounder.com/projects/elite-explorer-kit/ja/latest/basic_projects/06_basic_ultrasonic_sensor.html)
- [【Arduino】シリアルLED（WS2812B）を制御する](https://araisun.com/arduino-serial-led.html)
- [ArduinoでFFT解析し、ピークの周波数を検出する](https://qiita.com/ricelectric/items/98a6d32b1bcfca598762)
- [arduinoとprocessingの連携手順](https://note.com/nakariho/n/n92611b3c0046)
- [Audio MIDI設定ユーザガイド](https://support.apple.com/ja-jp/guide/audio-midi-setup/ams1013/mac)
