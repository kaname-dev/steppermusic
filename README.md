# Stepper Music - MIDI Controller

Arduino Nanoで最大4台のステッピングモーターを駆動し、USB MIDIキーボードでリアルタイムにピアノ全音階を演奏するシステムです。

## システムの特徴

- **ピアノ全88鍵対応**: C0(16.35Hz) ～ B8(7040Hz)
- **最大4音同時駆動**: 4台のステッピングモーター独立制御
- **リアルタイムMIDI入力**: Web MIDI API対応、USB MIDIキーボード直結
- **Web UIコントローラ**: ブラウザから接続・制御

## 必要な物

### ハードウェア
- Arduino Nano
- デュアルチャネルモータードライバ ×2 (L298N、TB6612など)
- ステッピングモーター ×4台 (NEMA17推奨)
- USB ケーブル (Arduino用)
- 外部電源 (モージドライバ用、12V 2A程度)
- ジャンパーワイヤ

### ソフトウェア
- Chrome または Edge ブラウザ (Web Serial API/MIDI API対応)
- Arduino IDE
- USB MIDIキーボード

## 配線図

### Arduino Nano ピン割り当て

```
Driver #1 (Motor A):
  IN1 -> D2
  IN2 -> D3
  IN3 -> D4
  IN4 -> D5

Driver #2 (Motor B):
  IN1 -> D8
  IN2 -> D9
  IN3 -> D10
  IN4 -> D11

追加の2モーター用:
  Motor 3: D12, D13, D6, D7
  Motor 4: A0, A1, A2, A3

シリアル通信:
  TX (D1) -> USB
  RX (D0) -> USB
  GND -> 共通グラウンド
```

### ステッピングモーター結線

各モーターの2つのコイル (A-phase, B-phase) を対応するドライバ出力に接続:

```
Motor coil A+ --- Driver OUT1
Motor coil A- --- Driver OUT2
Motor coil B+ --- Driver OUT3
Motor coil B- --- Driver OUT4
```

## セットアップ手順

### 1. Arduino に スケッチをアップロード

1. Arduino IDEで `steppermusic.ino` を開く
2. ボード選択: **Arduino Nano**
3. プロセッサ: **ATmega328P (Old Bootloader)** または確認
4. COMポートを選択
5. アップロード

### 2. HTML コントローラを開く

1. `midi_controller.html` をブラウザで開く (ChromeまたはEdge推奨)
   - ローカルファイルで OK（Web API も動作）
   - または、簡易サーバで実行: `python -m http.server 8000` → `http://localhost:8000/midi_controller.html`

### 3. Arduinoを接続

1. HTMLページの「Arduino接続」ボタンをクリック
2. ポップアップで Serialポートを選択
   - 通常: `COM3`, `COM4` など
   - 見分けがつかない場合は、Arduino接続/未接続で比較
3. 接続成功 (ステータス緑)

### 4. MIDIキーボードを接続

1. USB MIDIキーボードをコンピュータに接続
2. HTMLページのドロップダウンからMIDIデバイスを選択
3. キーボード入力でモーターが動く

## 使用方法

- **演奏**: USB MIDIキーボードの鍵盤を押す
  - 個々のモーターが対応周波数で自動駆動
- **同時演奏**: ポリフォニー有効で最大4音同時
- **リセット**: 「リセット」ボタンで全モーター停止
- **デバッグ**: ブラウザのコンソール (F12) でメッセージ確認

## シリアル通信プロトコル

Arduino は以下のコマンドをシリアルから受け取ります:

| コマンド | データ | 説明 |
|---------|--------|------|
| `0x90` | MIDI note (0-87) | ノート演奏開始 (次空きモーター) |
| `0x80` | モーター番号 (0-3) | ノート停止 |
| `0xFF` | - | 全ストップ |

例:
```c
// C4を演奏
Serial.write(0x90);    // コマンド
Serial.write(60);      // MIDI note C4
```

## 周波数対応表 (MIDI)

- MIDI note 0 = C-1 (8.18 Hz)
- MIDI note 12 = C0 (16.35 Hz)
- MIDI note 21 = A0 (27.50 Hz) ← ピアノ最低音
- MIDI note 69 = A4 (440 Hz)
- MIDI note 108 = C8 (4186 Hz) ← ピアノ最高音

## トラブルシューティング

### モーターが鳴らない

1. **配線確認**
   - Arduinoとドライバの接続、コイル配線を確認
   - GND が共通化されているか確認
2. **電源確認**
   - 外部電源がドライバに正しく供給されているか
   - 電源容量が不足していないか (推奨 12V 2A以上)
3. **ボーレート**
   - Arduino と HTML 両方で 115200 baud か確認

### 音程がずれる

- 周波数が低い場合: `stepTable` を半ステップシーケンスに変更
  ```c
  // 8-step half-step (microstepping)
  const uint8_t stepTable[8][4] = {
    {1, 0, 1, 0},
    {1, 0, 1, 1},
    {0, 0, 1, 1},
    ...
  };
  ```

### MIDIキーボードが認識されない

- ブラウザがWeb MIDI API に対応しているか確認 (Chrome/Edge)
- MIDIデバイスをONにしてから接続
- ドロップダウンを再度確認

### Arduino接続ができない

- Chrome/Edge を使用しているか確認
- COMポートが既に他のアプリで使用されていないか確認
- ドライバを確認 (CH340など)

## カスタマイズ

### ピン割り当てを変更

[steppermusic.ino](steppermusic.ino) の先頭:
```c
const uint8_t motorPins[4][4] = {
  {2, 3, 4, 5},      // Motor 1: D2,D3,D4,D5 を D12,D13,D6,D7 に変更など
  {8, 9, 10, 11},    // Motor 2
  {12, 13, 6, 7},    // Motor 3
  {A0, A1, A2, A3}   // Motor 4
};
```

### ステップ方式を変更

半ステップ（マイクロステップング）やフルステップなど:
```c
const uint8_t stepTable[N][4] = { ... };
```

### ボーレート変更

Arduino:
```c
Serial.begin(115200);  // 別のレートに変更
```

HTML:
```javascript
await serialPort.open({ baudRate: 115200 });  // 同じレート
```

## ライセンス

自由に使用・改造できます。

## 参考リンク

- [Web Serial API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API)
- [Web MIDI API](https://developer.mozilla.org/en-US/docs/Web/API/Web_MIDI_API)
- [Arduino Nano ピンアウト](https://store.arduino.cc/arduino-nano)

---

問題や改善提案があれば、お気軽にどうぞ！
