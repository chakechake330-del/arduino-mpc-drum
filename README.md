# 卒業制作2025
![デモ画像](images_and_videos/IMG_0586.jpeg)

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

[![Demo Video](https://img.youtube.com/vi/bGjGJwbTedI/0.jpg)](https://youtu.be/bGjGJwbTedI)

### *主な機能*

- **超音波モジュール**  
  センサーの前に手をかざすと、距離に応じたMIDIノートを生成し、Logic Proでドラム音や効果音を再生します。
　
- **Logic Pro**  
  ArduinoからのMIDI信号をLogic Proで受信し、ソフトウェア音源を鳴らします

- **FFT解析（PC側）**  
  Logic Proの音声出力をPCで取得し、リアルタイムFFT解析を実行します

- **WS2812B LEDマトリクス**  
  解析結果に基づき、8×32のLEDマトリクスにスペクトラムアナライザを表示します。

- **Arduino Uno R4 WiFi**  
  超音波センサーの読み取り、MIDIノートの送信、LED制御を1台で実行します。

### *システムデータフロー*

>このプロジェクトでは、以下の4つのステップでデータがリアルタイムに循環しています。

1.  Arduino Uno R4 WiFiが超音波センサーを用いて、手との距離をリアルタイムで読み取ります。

2.  読み取った距離情報をMIDI信号に変換し、IAC Driver 経由で Logic Pro へ送信してソフト音源を再生します。

3.  Logic Proから出力された音声を、仮想オーディオドライバ(BlackHole) を通してPC内の Javaアプリケーション（IntelliJ IDEA） へ送ります。

4.  Javaアプリケーション が音声をFFT解析し、その解析結果をシリアル通信で再びArduinoへ送信することで、LEDマトリクス を点灯させます。

## 3.  仕様書

### *配線図*
> ※配線図にはブレッドボード用の電源モジュールは含まれていませんが、このプロジェクトでは電源モジュールの使用することをおすすめします。  
> ※この配線図はFritzingの都合により、以下の部品を代用品で表現しています
> - LEDマトリクス：実際には8×32のWS2812Bマトリクス（データピン：D6）を使用

![配線図](images_and_videos/Arduino_drum_配線図.png)

### *回路図*

![回路図]()

### *使用モジュールとピン*

| モジュール名                 | 用途                                | 使用ピン（Arduino Uno R4 WiFi）  |
|---------------------------|-------------------------------------|------------------------------|
| 超音波距離センサー（HC-SR04など） | 手の距離を測定し、MIDIノートを決定        |      TRIG: D10 / ECHO: D9    |
| WS2812B LEDマトリクス（8×32）  | スペクトラムアナライザ表示               | データピン: D6                  |
| Arduino Uno R4 WiFi         | 全体制御（MIDI送信・LED制御）         | -                             |
| PC（Logic Pro + Processing）     | 音声再生・FFT解析・LED制御         | USBシリアル通信                  |

### 使用ツール・環境

| ツール名 | 用途 | 詳細 |
| :--- | :--- | :--- |
| **IntelliJ IDEA** | Java開発環境 | メインのアプリケーション開発。FFT解析やシリアル通信の制御プログラムを作成。 |
| **Arduino IDE** | マイコン開発環境 | Arduino Uno R4 WiFiへのコード開発および書き込みに使用。 |
| **Logic Pro** | DAW（音楽制作ソフト） | MIDI信号を受信し、ドラムやシンセサイザーなどの音源を再生。 |
| **BlackHole** | 仮想オーディオドライバ | Logic Proの音声をJavaアプリへ送るための仮想配線として使用。 |
| **Audio MIDI設定** | 仮想MIDIツール | Mac標準機能（IACドライバ）を使用し、仮想MIDIポートを作成。 |

### Arduino使用ライブラリ

| ライブラリ名 | 標準 / 外部 | 用途・詳細 |
| :--- | :--- | :--- |
| **Adafruit_NeoPixel** | 外部ライブラリ | **LED制御**<br>WS2812B LEDマトリクスの制御 |


### IntelliJ IDEA使用ライブラリ

| ライブラリ名 | 標準 / 外部 | 用途・詳細 |
| :--- | :--- | :--- |
| **jSerialComm** | 外部ライブラリ | **Arduinoとのシリアル通信**<br>距離データの受信、およびLED制御データの送信に使用。 |
| **Java Sound API** | 標準ライブラリ | **MIDI信号・音声処理**<br>IACドライバと併用し、MIDI信号の送信や音声の取得を管理。 |
| **FFT（JTransforms等）** | 外部または自作 | **FFT解析**<br>音声をリアルタイムで周波数解析し、スペクトラム情報を取得。 |

## 4.  システムブロック図
![ブロック図](images_and_videos/mermaid-diagram-2025-12-25-113710.png)

## 5.  フローチャート

###  *[MIDI楽器コントローラ(Arduino側)]*

```mermaid
flowchart TD
    %% 起動プロセス
    START([システム起動]) --> INIT[初期化<br/>センサー・LEDの準備]
    INIT --> CONNECT[PCとの通信開始]
    
    %% メイン処理の枠組み
    subgraph LOOP [リアルタイム処理ループ]
        direction TB
        
        %% 入力セクション
        SENSOR[超音波センサーで距離計測]
        CALC[距離をcmデータに変換]
        SEND_PC[PCへ送信<br/><b>音を鳴らすためのデータ</b>]
        
        %% 受信・出力セクション
        RECV_PC[PCからデータ受信<br/><b>音の解析結果</b>]
        CONV_LED[LED表示データに変換]
        LIGHT[LEDマトリクス点灯]
        
        %% 流れの定義
        SENSOR --> CALC
        CALC --> SEND_PC
        SEND_PC --> RECV_PC
        RECV_PC --> CONV_LED
        CONV_LED --> LIGHT
    end
    
    %% ループの戻り
    LIGHT -.->|高速でループ（繰り返し）| SENSOR

    %% デザインのカスタマイズ
    style START fill:#f9f,stroke:#333
    style SEND_PC fill:#fff9c4,stroke:#fbc02d
    style RECV_PC fill:#e1f5fe,stroke:#0288d1
    style LOOP fill:#fafafa,stroke:#999,stroke-dasharray: 5 5
```

###  *[音響解析・映像生成(IntelliJ IDEA側)]*

```mermaid
flowchart TD
    %% 全体の流れを定義
    START([システム起動]) --> INIT[初期化: 音声・通信・MIDI]
    INIT --> LOOP_START{リアルタイム処理}

    %% 音声解析セクション
    subgraph AudioAnalysis [音声の可視化処理]
        LISTEN[Logic Proの音を聴く] --> FFT[FFT解析: 周波数に分解]
        FFT --> LED_DATA[LED用の表示データを作成]
    end

    %% センサーとMIDIセクション
    subgraph SensorMidi [楽器としての制御]
        RECV[距離データを受信] --> CHECK{動きがあるか?}
        CHECK -- Yes --> MIDI[MIDI送信: 音を鳴らす]
        CHECK -- No --> SKIP[待機]
    end

    %% 統合と送信
    LOOP_START --> LISTEN
    LOOP_START --> RECV
    
    LED_DATA --> SEND[Arduinoへ点灯指示を送信]
    MIDI --> SEND
    SKIP --> SEND
    
    SEND -->|最新の状態へ更新| LOOP_START

    %% 見栄えの調整
    style AudioAnalysis fill:#e1f5fe,stroke:#01579b
    style SensorMidi fill:#fff9c4,stroke:#fbc02d
    style START fill:#f9f,stroke:#333
```


## 6.  使用ツールの詳細

### 🔹 IntelliJ IDEA
Javaを用いて、Arduinoから届くデータの解析や、Logic Proへ送るMIDI信号の生成、さらに音声をリアルタイムで分析するFFT処理など、システム全体の中心的な計算を行っています。

![デモ画面](images_and_videos/IntelliJ_IDEA.png)

### 🔹 BlackHole
macOS用の仮想オーディオドライバです。通常、アプリの音声はスピーカーへ送られますが、BlackHoleを経由させることで、Logic Proの音声をリアルタイムでIntelliJに取り込むことができます。

### 🔹 Audio MIDI設定（IACドライバ）
macOS標準のアプリとアプリをつなぐ「見えない演奏用ケーブル」です。IntelliJで生成した「ドレミ」などの演奏情報を、仮想MIDIポートを通じてLogic Proへ送信し、実際に楽器を鳴らすために使用しています。

## 7.  工夫ポイント

### ◎ ArduinoとIntelliJのリアルタイムやりとり

センサー（入力）とLED（出力）を同時に高速処理。  
Arduinoは「距離を測る」と「光らせる」に専念させ、複雑な計算はPC（IntelliJ）に任せるという役割分担を徹底することで、  
遅延のないスムーズな操作感を実現しました。

### ◎ リアルタイムFFT解析

単なる音量反応ではなく「音の高さ」を可視化。  
Logic Proの音をBlackHoleでIntelliJに取り込み、その場で周波数解析（FFT）を行っています。  

## 8.  参考サイト

- [基本プロジェクト 超音波](https://docs.sunfounder.com/projects/elite-explorer-kit/ja/latest/basic_projects/06_basic_ultrasonic_sensor.html)
- [【Arduino】シリアルLED（WS2812B）を制御する](https://araisun.com/arduino-serial-led.html)
- [ArduinoでFFT解析し、ピークの周波数を検出する](https://qiita.com/ricelectric/items/98a6d32b1bcfca598762)
- [Audio MIDI設定ユーザガイド](https://support.apple.com/ja-jp/guide/audio-midi-setup/ams1013/mac)
