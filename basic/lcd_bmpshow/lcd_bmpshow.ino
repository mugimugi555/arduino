//arduino-cli lib install MCUFRIEND_kbv
//arduino-cli lib install "Adafruit GFX Library"
//arduino-cli lib install "Adafruit TFTLCD Library"

// https://stephenmonro.wordpress.com/2023/11/20/jaycar-arduino-2-8-uno-module/

//arduino-cli lib install MCUFRIEND_kbv
//arduino-cli lib install "Adafruit GFX Library"
//arduino-cli lib install "Adafruit TFTLCD Library"

// https://stephenmonro.wordpress.com/2023/11/20/jaycar-arduino-2-8-uno-module/

#include <SPI.h>            // SPIライブラリをインクルード
//#define USE_SDFAT
#include <SD.h>             // ハードウェアピン用の公式SDライブラリを使用

#include <Adafruit_GFX.h>   // ハードウェア特有のライブラリ
#include <MCUFRIEND_kbv.h>  // TFT LCD用のライブラリ
MCUFRIEND_kbv tft;           // TFT LCDオブジェクトを作成

#if defined(ESP32)
#define SD_CS     10 // JAYCAR TFTスクリーン用に変更
#else
#define SD_CS     10 // JAYCAR TFTスクリーン用に変更
#endif
#define NAMEMATCH ""        // "" は任意の名前にマッチ
//#define NAMEMATCH "tiger"   // *tiger*.bmp
#define PALETTEDEPTH   0     // パレットモードをサポートしない
//#define PALETTEDEPTH   8     // 256色パレットをサポート

char namebuf[32] = "/";   // ルートディレクトリのBMPファイル
//char namebuf[32] = "/bitmaps/";  // 例: /bitmaps/*.bmp のBMPディレクトリ

File root;                // SDカードのルートファイル
int pathlen;             // パスの長さ

void setup() {
  uint16_t ID;           // デバイスIDを格納する変数
  Serial.begin(9600);   // シリアル通信を9600ボーレートで開始
  Serial.print("Show BMP files on TFT with ID:0x");
  ID = tft.readID();    // TFTのIDを読み取る
  Serial.println(ID, HEX); // IDを16進数で表示
  if (ID == 0x0D3D3) ID = 0x9481; // IDの調整
  tft.begin(ID);        // TFTを初期化
  tft.fillScreen(0x001F); // 画面を青で塗りつぶす
  tft.setTextColor(0xFFFF, 0x0000); // テキストの色を白に設定
  bool good = SD.begin(SD_CS); // SDカードを初期化
  if (!good) {
    Serial.print(F("cannot start SD")); // SDカードの初期化に失敗
    while (1); // 無限ループで停止
  }
  root = SD.open(namebuf); // ルートディレクトリをオープン
  pathlen = strlen(namebuf); // パスの長さを取得
}

void loop() {
  char *nm = namebuf + pathlen; // 名前バッファのポインタを設定
  File f = root.openNextFile(); // 次のファイルをオープン
  uint8_t ret;                  // 結果を格納する変数
  uint32_t start;              // 処理開始時刻を格納する変数
  if (f != NULL) {             // ファイルが存在する場合
#ifdef USE_SDFAT
    f.getName(nm, 32 - pathlen); // 名前を取得
#else
    strcpy(nm, (char *)f.name()); // 名前をコピー
#endif
    f.close();                // ファイルを閉じる
    strlwr(nm);              // 名前を小文字に変換
    if (strstr(nm, ".bmp") != NULL && strstr(nm, NAMEMATCH) != NULL) { // BMPファイルかチェック
      Serial.print(namebuf); // ファイル名を表示
      Serial.print(F(" - "));
      tft.fillScreen(0); // 画面をクリア
      start = millis(); // 処理開始時刻を記録
      ret = showBMP(namebuf, 5, 5); // BMPファイルを表示
      switch (ret) { // 結果に応じて処理
        case 0:
          Serial.print(millis() - start); // 処理時間を表示
          Serial.println(F("ms"));
          delay(5000); // 5秒待機
          break;
        case 1:
          Serial.println(F("bad position")); // 位置が不正
          break;
        case 2:
          Serial.println(F("bad BMP ID")); // BMP IDが不正
          break;
        case 3:
          Serial.println(F("wrong number of planes")); // プレーン数が不正
          break;
        case 4:
          Serial.println(F("unsupported BMP format")); // BMPフォーマットがサポートされていない
          break;
        case 5:
          Serial.println(F("unsupported palette")); // パレットがサポートされていない
          break;
        default:
          Serial.println(F("unknown")); // 不明なエラー
          break;
      }
    }
  } else {
    root.rewindDirectory(); // ルートディレクトリを巻き戻す
  }
}

#define BMPIMAGEOFFSET 54 // BMP画像のオフセット
#define BUFFPIXEL      20 // バッファピクセルサイズ

uint16_t read16(File& f) {
  uint16_t result;         // 16ビットの結果を格納
  f.read((uint8_t*)&result, sizeof(result)); // 16ビットデータを読み込む
  return result;          // 結果を返す
}

uint32_t read32(File& f) {
  uint32_t result;        // 32ビットの結果を格納
  f.read((uint8_t*)&result, sizeof(result)); // 32ビットデータを読み込む
  return result;          // 結果を返す
}

uint8_t showBMP(char *nm, int x, int y) {
  File bmpFile;                  // BMPファイルを扱うためのFileオブジェクト
  int bmpWidth, bmpHeight;      // BMPの幅と高さ（ピクセル単位）
  uint8_t bmpDepth;             // ビット深度（現在は24, 16, 8, 4, 1のみ対応）
  uint32_t bmpImageoffset;      // 画像データの開始位置
  uint32_t rowSize;             // 各行のサイズ（パディングを含む場合がある）
  uint8_t sdbuffer[3 * BUFFPIXEL]; // ピクセルデータをバッファに格納（R+G+B）
  uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL; // LCD用バッファとカラーパレット
  uint8_t bitmask, bitshift;    // パレットのビットマスクとシフト量
  boolean flip = true;          // BMPが下から上に保存されているかどうか
  int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx; // 各種インデックス
  uint32_t pos;                 // シーク位置
  boolean is565 = false;        // 565フォーマットかどうか

  uint16_t bmpID;               // BMPファイルのID
  uint16_t n;                   // 読み取ったブロック数
  uint8_t ret;                  // 戻り値

  // 描画位置が画面外であればエラーを返す
  if ((x >= tft.width()) || (y >= tft.height()))
    return 1;                // 画面外

  bmpFile = SD.open(nm);       // BMPヘッダーの解析
  bmpID = read16(bmpFile);     // BMPシグネチャを読み取る
  (void) read32(bmpFile);      // ファイルサイズを読み取って無視
  (void) read32(bmpFile);      // 作成者バイトを読み取って無視
  bmpImageoffset = read32(bmpFile); // 画像データの開始位置を取得
  (void) read32(bmpFile);      // DIBヘッダーサイズを読み取って無視
  bmpWidth = read32(bmpFile);  // 画像の幅を取得
  bmpHeight = read32(bmpFile); // 画像の高さを取得
  n = read16(bmpFile);         // プレーン数を取得（'1'である必要がある）
  bmpDepth = read16(bmpFile);  // ピクセルあたりのビット数を取得
  pos = read32(bmpFile);       // フォーマットを取得

  // BMPファイルの検証
  if (bmpID != 0x4D42) {
      ret = 2; // 不正なID
  } else if (n != 1) {
      ret = 3; // プレーンが多すぎる
  } else if (pos != 0 && pos != 3) {
      ret = 4; // フォーマットが不正
  } else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) {
      ret = 5; // パレットのビット深度が不正
  } else {
      bool first = true;          // 初回フラグ
      is565 = (pos == 3);        // 16ビットフォーマットかどうか

      // BMP行は4バイト境界にパディングされる
      rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;

      // 高さが負の場合、画像は上から下に保存されている
      if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip = false;          // フリップフラグをオフ
      }

      w = bmpWidth;              // 幅を設定
      h = bmpHeight;             // 高さを設定

      // 描画する領域を画面のサイズにクリップ
      if ((x + w) >= tft.width()) {
          w = tft.width() - x;   // 幅が画面外であれば調整
      }
      if ((y + h) >= tft.height()) {
          h = tft.height() - y;  // 高さが画面外であれば調整
      }

      // パレットを使用する場合の処理
      if (bmpDepth <= PALETTEDEPTH) {
          // カラーパレットを読み取る
          bmpFile.seek(bmpImageoffset - (4 << bmpDepth)); // カラーパレットの位置に移動
          bitmask = 0xFF; // ビットマスクの初期化
          if (bmpDepth < 8) {
              bitmask >>= bmpDepth; // ビットマスクをシフト
          }
          bitshift = 8 - bmpDepth; // シフト量を計算
          n = 1 << bmpDepth;        // パレットのエントリー数を計算
          lcdbufsiz -= n;          // LCDバッファサイズを調整
          palette = lcdbuffer + lcdbufsiz; // パレットの位置を設定

          // パレットの読み込み
          for (col = 0; col < n; col++) {
              pos = read32(bmpFile); // パレットをマップ
              palette[col] = ((pos & 0x0000F8) >> 3) |
                              ((pos & 0x00FC00) >> 5) |
                              ((pos & 0xF80000) >> 8);
          }
      }

      // TFTアドレスウィンドウをクリップされた画像の範囲に設定
      tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

      for (row = 0; row < h; row++) { // 各スキャンラインに対して...
          // スキャンラインの開始位置にシーク
          uint8_t r, g, b, *sdptr;     // RGBの色成分とポインタを初期化
          int lcdidx, lcdleft;        // LCDバッファインデックスと残りのピクセル数

          // BMPが下から上に保存されている場合（通常のBMP）
          if (flip) {
              pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize; // 逆順に位置を計算
          } else {
              pos = bmpImageoffset + row * rowSize; // 正順に位置を計算
          }

          // シークが必要か？
          if (bmpFile.position() != pos) { // ファイルポジションが必要な場合
              bmpFile.seek(pos); // 指定位置にシーク
              buffidx = sizeof(sdbuffer); // バッファを強制的に再読み込み
          }

          for (col = 0; col < w; ) {  // 行のピクセル数に対して
              lcdleft = w - col;      // 残りのピクセル数を計算
              if (lcdleft > lcdbufsiz) {
                  lcdleft = lcdbufsiz; // バッファサイズを超えないように調整
              }

              for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // バッファ単位で処理
                  uint16_t color; // ピクセルの色を格納する変数

                  // ピクセルデータをさらに読み取る必要があるか？
                  if (buffidx >= sizeof(sdbuffer)) { // 必要な場合
                      bmpFile.read(sdbuffer, sizeof(sdbuffer)); // バッファに読み込む
                      buffidx = 0; // インデックスを初期化
                      r = 0; // 赤色の初期化
                  }

                  // BMPからTFTフォーマットにピクセルを変換
                  switch (bmpDepth) {
                      case 32:
                      case 24:
                          b = sdbuffer[buffidx++]; // 青色を取得
                          g = sdbuffer[buffidx++]; // 緑色を取得
                          r = sdbuffer[buffidx++]; // 赤色を取得
                          if (bmpDepth == 32) {
                              buffidx++; // アルファチャネルを無視
                          }
                          color = tft.color565(r, g, b); // RGBから565フォーマットに変換
                          break;
                      case 16:
                          b = sdbuffer[buffidx++]; // 青色を取得
                          r = sdbuffer[buffidx++]; // 赤色を取得
                          if (is565) {
                              color = (r << 8) | (b); // 565フォーマットの場合
                          } else {
                              color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F); // それ以外のフォーマットの場合
                          }
                          break;
                      case 1:
                      case 4:
                      case 8:
                          if (r == 0) {
                              b = sdbuffer[buffidx++], r = 8; // 初回読み込みの場合
                          }
                          color = palette[(b >> bitshift) & bitmask]; // パレットから色を取得
                          r -= bmpDepth; // カウンターを減少
                          b <<= bmpDepth; // シフト
                          break;
                  }

                  lcdbuffer[lcdidx] = color; // LCDバッファに色を保存
              }
              // LCDにバッファを送信
              tft.pushColors(lcdbuffer, lcdidx, first);
              first = false; // 初回フラグをオフ
              col += lcdidx; // 列のインデックスを更新
          } // 列のループ終了
      } // 行のループ終了

      // フルスクリーンに戻す
      tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1);
      ret = 0; // 正常に描画された

    }
    bmpFile.close();
    return (ret);
}