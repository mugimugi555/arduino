#include <WiFi.h>                 // ESP32用のWiFiライブラリ
#include <ESPmDNS.h>             // mDNS（マルチキャストDNS）を使用するためのライブラリ
#include <ESPAsyncWebServer.h>   // 非同期Webサーバーライブラリ
#include <ArduinoJson.h>         // ArduinoJsonライブラリ

const char* ssid     = "WIFISSID";   // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME";   // ESP32のホスト名

AsyncWebServer server(80); // ポート80で非同期Webサーバーを初期化

String jsonResponse;

void setup() {
    Serial.begin(115200);
    showStartupScreen();
    connectToWiFi();
    setupWebServer();
}

void loop() {
    // ハードウェア情報を取得してJSONレスポンスを生成
    createJsonResponse(jsonResponse);

    // シリアルに出力
    Serial.println(jsonResponse);

    // 6秒待機
    delay(6000);
}

//----------------------------------------------------------------------------
// スプラッシュ画面の表示
//----------------------------------------------------------------------------
void showStartupScreen() {
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
// WiFi接続関数
//----------------------------------------------------------------------------
void connectToWiFi() {
    WiFi.hostname(hostname);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to ");
    Serial.print(ssid);

    // WiFi接続が完了するまで待機
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");

    // mDNSサービスの開始
    if (MDNS.begin(hostname)) {
        Serial.println("mDNS responder started");
    } else {
        Serial.println("Error setting up mDNS responder!");
    }

    Serial.println("");
    Serial.println("===============================================");
    Serial.println("              Network Details                  ");
    Serial.println("===============================================");
    Serial.print("WebServer    : http://");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname     : http://");
    Serial.print(hostname);
    Serial.println(".local");
    Serial.print("IP address   :");
    Serial.println(WiFi.localIP());
    Serial.print("Subnet Mask  :");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway IP   :");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS IP       :");
    Serial.println(WiFi.dnsIP());
    Serial.print("MAC address  :");
    Serial.println(WiFi.macAddress());
    Serial.println("-----------------------------------------------");
    Serial.println("");
}

//----------------------------------------------------------------------------
// Webサーバーの設定
//----------------------------------------------------------------------------
void setupWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String jsonResponse;
        createJsonResponse(jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    server.begin(); // Webサーバーを開始
}

//----------------------------------------------------------------------------
// JSONレスポンスを生成
//----------------------------------------------------------------------------
void createJsonResponse(String &response) {
    StaticJsonDocument<512> doc; // JSONドキュメントのサイズを設定

    // ボード名を表示
    doc["Board"] = "ESP32";

    // CPUの周波数を表示
    doc["CPU Frequency (MHz)"] = ESP.getCpuFreqMHz();

    // フラッシュサイズを表示
    doc["Flash Size (KB)"] = ESP.getFlashChipSize() / 1024;

    // 空きヒープメモリを表示
    doc["Free Heap (B)"] = ESP.getFreeHeap();

    // Wi-Fi情報を追加
    doc["SSID"] = WiFi.SSID();
    doc["RSSI (dBm)"] = WiFi.RSSI();
    doc["IP Address"] = WiFi.localIP().toString();
    doc["MAC Address"] = WiFi.macAddress();

    // フラッシュ速度を取得
    doc["Flash Speed (MHz)"] = ESP.getFlashChipSpeed();

    // チップIDを取得
    doc["Chip ID"] = ESP.getChipId();

    // SDKバージョンを取得
    doc["SDK Version"] = ESP.getSdkVersion();

    // JSON文字列を生成
    serializeJson(doc, response);
}
