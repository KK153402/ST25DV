# ST25DV NDEF初期化プログラム

ST25DVはSTMicroelectronics製の高機能NFC Dynamic Tagです。
出荷時はNDEF（NFC Data Exchange Format）構造が初期化されていないため、NFCToolsでデータの書き込みができません。
このリポジトリでは、ESP32 + PlatformIOを使ってNDEF初期化を行うプログラムを保存しています。

> 参考記事: [ST25DVにNDEF初期化を書き込む（note）](https://note.com/brainy_clover252/n/nc2a47c27a467)

---

## 必要なもの

| 項目 | 内容 |
|------|------|
| マイコン | ESP32 |
| NFCチップ | ST25DV（ST25DV64K） |
| 開発環境 | PlatformIO（Arduino framework） |
| 確認アプリ | NFCTools（スマートフォン） |
| ライブラリ | Wire.h（I2C通信、標準搭載） |

---

## 配線

| ESP32 | ST25DV |
|-------|--------|
| GPIO21 (SDA) | SDA |
| GPIO22 (SCL) | SCL |
| 3.3V | VCC |
| GND | GND |

I2Cクロック: 100kHz

---

## ST25DV I2Cアドレス

| アドレス | 用途 |
|----------|------|
| `0x53` | ユーザーメモリ（読み書き） |
| `0x57` | システムレジスタ |

---

## 処理の流れ

1. I2C初期化（SDA: GPIO21、SCL: GPIO22）
2. ST25DV接続確認（0x53 / 0x57）
3. 初期化前のメモリ内容をシリアルモニタに出力
4. **CC Fileを書き込む**（アドレス `0x0000`）
5. **空のNDEFメッセージを書き込む**（アドレス `0x0008`）
6. 初期化後のメモリ内容を確認して成否を判定

### CC File の内容

| バイト | 値 | 意味 |
|--------|----|------|
| 0 | `0xE1` | Magic Number（NDEF準拠） |
| 1 | `0x40` | Version 1.0、Read/Write可 |
| 2 | `0x40` | メモリサイズ（64Kbit） |
| 3 | `0x00` | 追加フラグ |
| 4 | `0x03` | TLV: NDEF Message |
| 5 | `0x00` | Length MSB |
| 6 | `0x03` | Length LSB（3バイト） |
| 7 | `0x00` | 予約 |

---

## シリアルモニタ出力例（成功時）

```
=== ST25DV NDEF初期化 ===
接続確認中...
✅ ST25DV接続成功 (0x53)
✅ ST25DV接続成功 (0x57)

--- 初期化前のメモリ ---
FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF

--- NDEF初期化実行 ---
✅ CC File書き込み完了
✅ 空のNDEFメッセージ作成

--- 初期化後のメモリ ---
E1 40 40 00 03 00 03 00
D0 00 00 FF FF FF FF FF

✅✅✅ NDEF初期化成功! ✅✅✅
```

---

## 初期化後の使い方

1. スマホのNFCToolsを起動
2. 基板をスマホにタッチ
3. 「空のタグ」として認識される
4. URLやテキストを書き込める

---

## プログラムファイル

- `ST25DV_NDEF設定用.txt` — 初期化プログラム本体（PlatformIO / Arduino framework）
