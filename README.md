# 卒業制作2025
## 1.  概要 
  超音波センサー × Logic Pro × LEDマトリクスで構成された、 手の動きで音を奏で、音に合わせてLEDが光る **エアー楽器**  

[![Demo](images_and_videos/thumbnail.jpg)](https://chakechake330-del.github.io/arduino_air_musical_instrument/)

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

### *システム構成*

1.  Arduino Uno R4 WiFi が超音波センサーで距離を測定し、MIDIノートをUSB経由で送信
2.  Logic Pro が IAC Driver 経由でMIDIを受信し、ソフト音源を再生
3.  Logic Proの音声出力 を BlackHole 経由で Processing に渡す
4.  Processing がFFT解析を行い、LEDマトリクスにスペクトラムを表示

## 2.  仕様書

### *配線図*
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
| Arduino Uno R4 WiFi         | 全体制御（MIDI送信・LED制御））         | -                             |
| PC（Logic Pro + Processing）     | 音声再生・FFT解析・LED制御         | USBシリアル通信                  |

### 使用ツール・環境

- **Arduino IDE**（統合開発環境 / マイコン用コードの開発・書き込み）
- **Processing バージョン：3.5.4（推奨）**（ビジュアルプログラミング環境 / 音声解析・シリアル通信・LED制御）
- **Logic Pro**（DAW / MIDI受信と音声出力）
- **BlackHole**（仮想オーディオルーティング / ProcessingでLogic Proの音を取得）
- **Audio MIDI設定（IACドライバ）**（Mac標準 / 仮想MIDIポートの作成・接続）

### Arduino使用ライブラリ

- **FastLED**(WS2812B LEDマトリクス制御用ライブラリ)

### Processing使用ライブラリ

- **Minim**(音声入力とFFT解析用ライブラリ)
- **TheMidiBus**(MIDIノート送信用ライブラリ)
- **processing.serial**(Arduinoとのシリアル通信)

## 3.  フローチャート

###  *[Arduino側]*

```mermaid
flowchart TD
    A[起動] --> B[センサーとLEDの初期化]
    B --> C[シリアル通信開始]
    C --> D[loop開始]

    D --> E[超音波センサーで距離を測定]
    E --> F[距離をcm単位で計算]
    F --> G[距離をシリアルでPCに送信]

    G --> H[PCからLEDデータを受信]
    H --> I[データを読み取り、LEDの光り方に変換]
    I --> J[縦の位置に応じた色でLEDを点灯]
    J --> D
```

###  *[Processing側]*

```mermaid
flowchart TD
    A[起動] --> B[音声入力とFFTの初期化]
    B --> C[シリアル通信とMIDIポートの初期化]
    C --> D[ループ開始]

    D --> E[Logic Proの音声を取得（BlackHole経由）]
    E --> F[FFT解析を実行]
    F --> G[音の変化を記録]
    G --> H[LEDマトリクス用データを生成]
    H --> I[ArduinoにLEDデータを送信]

    D --> J[Arduinoから距離データを受信]
    J --> K[前回の距離と比較]
    K --> L{変化が一定以上？}
    L -- Yes --> M[MIDIノートを計算して送信]
    L -- No --> N[スキップ]

    M --> D
    N --> D
```


## 4.  使用ツールの詳細

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


## 5.  工夫ポイント

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


## 6.  参考サイト

- [基本プロジェクト 超音波](https://docs.sunfounder.com/projects/elite-explorer-kit/ja/latest/basic_projects/06_basic_ultrasonic_sensor.html)
- [【Arduino】シリアルLED（WS2812B）を制御する](https://araisun.com/arduino-serial-led.html)
- [ArduinoでFFT解析し、ピークの周波数を検出する](https://qiita.com/ricelectric/items/98a6d32b1bcfca598762)
- [arduinoとprocessingの連携手順](https://note.com/nakariho/n/n92611b3c0046)
- [Audio MIDI設定ユーザガイド](https://support.apple.com/ja-jp/guide/audio-midi-setup/ams1013/mac)
