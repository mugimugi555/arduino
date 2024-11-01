#include <ArduinoJson.h>

void setup() {
    Serial.begin(115200);
    showSplash();
}

void loop() {
    // ハードウェア情報を取得してJSONレスポンスを生成
    String jsonResponse;
    createHardwareInfoJson(jsonResponse);

    // シリアルに出力
    Serial.println(jsonResponse);

    // 6秒待機
    delay(6000);
}

//----------------------------------------------------------------------------
// スプラッシュ画面の表示
//----------------------------------------------------------------------------
void showSplash() {
    Serial.println("");
    Serial.println("");
    Serial.println("===============================================");
    Serial.println("  _____ ____  ____  ___ ____   __    __");
    Serial.println("  | ____/ ___||  _ \\( _ )___ \\ / /_  / /_  ");
    Serial.println("  |  _| \\___ \\| |_) / _ \\ __) | '_ \\| '_ \\ ");
    Serial.println("  | |___ ___) |  __/ (_) / __/| (_) | (_) |");
    Serial.println("  |_____|____/|_|   \\___/_____|\\___/ \\___/ ");
    Serial.println("");
    Serial.println("===============================================");
    Serial.println("");
}

//----------------------------------------------------------------------------
// ハードウェア情報を取得してJSON形式で出力
//----------------------------------------------------------------------------
void createHardwareInfoJson(String &response) {
    StaticJsonDocument<512> doc; // JSONドキュメントのサイズを設定

    // ボード名を表示
    doc["Board"] = "Arduino"; // ここはArduinoのボード名を手動で入力

    // CPUの周波数を表示
    doc["CPU Frequency (MHz)"] = F_CPU / 1000000; // ArduinoのCPU周波数を取得

    // フラッシュサイズを表示（例として）
    doc["Flash Size (KB)"] = 32; // 実際のフラッシュサイズに置き換えてください

    // 空きヒープメモリを表示（例として）
    doc["Free Heap (B)"] = 2000; // 実際の空きメモリに置き換えてください

    // JSON文字列を生成
    serializeJson(doc, response);
}
