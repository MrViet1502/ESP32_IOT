
// // Import required libraries
// #include "WiFi.h"
// #include "ESPAsyncWebServer.h"
// #include "SPIFFS.h"
// #include "DHT20.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include <Arduino.h>
// #include <PubSubClient.h>  // Thư viện MQTT cho ESP32

// // Replace with your network credentials

// const char* ssid = "viet";
// const char* password = "20252025";

// // ThingsBoard Credentials
// const char* mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char* ACCESS_TOKEN = "viet";  // Thay bằng token thiết bị trên ThingsBoard

// WiFiClient espClient;
// PubSubClient client(espClient);
// DHT20 dht20;

// void connectWiFi() {
//     Serial.print("Connecting to WiFi...");
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED) {
//         Serial.print(".");
//         delay(1000);
//     }
//     Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
// }

// void reconnectMQTT() {
//     while (!client.connected()) {
//         Serial.print("Connecting to MQTT...");
//         if (client.connect("ESP32", ACCESS_TOKEN, "")) {
//             Serial.println("Connected to MQTT!");
//         } else {
//             Serial.print("Failed, rc=");
//             Serial.print(client.state());
//             Serial.println(" retrying in 5 seconds...");
//             delay(5000);
//         }
//     }
// }

// void sendTelemetry() {
//     if (!dht20.read()) {  // Đọc dữ liệu từ DHT20
//         float temperature = dht20.getTemperature();
//         float humidity = dht20.getHumidity();

//         // Tạo JSON để gửi dữ liệu lên ThingsBoard
//         String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
//         client.publish("v1/devices/me/telemetry", payload.c_str());

//         Serial.println("Sent telemetry: " + payload);
//     } else {
//         Serial.println("Failed to read DHT20 sensor!");
//     }
// }

// void setup() {
//     Serial.begin(115200);
//     Wire.begin();
//     dht20.begin();

//     connectWiFi();
//     client.setServer(mqttServer, mqttPort);
// }

// void loop() {
//     if (!client.connected()) {
//         reconnectMQTT();
//     }
//     client.loop();

//     sendTelemetry();  // Gửi dữ liệu cảm biến mỗi 5 giây
//     delay(5000);
// }

//============================// led
// // Set LED GPIO
// const int ledPin = 18;
// // Stores LED state
// String ledState;

// // Create AsyncWebServer object on port 80
// AsyncWebServer server(80);

// // Replaces placeholder with LED state value
// String processor(const String& var){
//   Serial.println(var);
//   if(var == "STATE"){
//     if(digitalRead(ledPin)){
//       ledState = "ON";
//     }
//     else{
//       ledState = "OFF";
//     }
//     Serial.print(ledState);
//     return ledState;
//   }
//   return String();
// }

// // Task to handle Wi-Fi connection
// void wifiTask(void *pvParameters) {
//   Serial.begin(115200);
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//     Serial.println("Connecting to WiFi..");
//   }

//   // Print ESP32 Local IP Address
//   Serial.println(WiFi.localIP());
//   vTaskDelete(NULL);  // Delete the task when done
// }

// // Task to handle server
// void serverTask(void *pvParameters) {
//   // Initialize SPIFFS
//   if(!SPIFFS.begin(true)){
//     Serial.println("An Error has occurred while mounting SPIFFS");
//     vTaskDelete(NULL);  // Delete the task if SPIFFS initialization fails
//   }

//   // Route for root / web page
//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//     request->send(SPIFFS, "/index.html", String(), false, processor);
//   });

//   // Route to load style.css file
//   server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
//     request->send(SPIFFS, "/style.css", "text/css");
//   });

//   // Route to set GPIO to HIGH
//   server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
//     digitalWrite(ledPin, HIGH);
//     request->send(SPIFFS, "/index.html", String(), false, processor);
//   });

//   // Route to set GPIO to LOW
//   server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
//     digitalWrite(ledPin, LOW);
//     request->send(SPIFFS, "/index.html", String(), false, processor);
//   });

//   // Start server
//   server.begin();
//   vTaskDelete(NULL);  // Delete the task when done
// }

// void setup(){
//   Serial.begin(115200);
//   pinMode(ledPin, OUTPUT);

//   // Create tasks for Wi-Fi and server
//   xTaskCreate(wifiTask, "WiFiTask", 4096, NULL, 1, NULL);
//   xTaskCreate(serverTask, "ServerTask", 8192, NULL, 1, NULL);
// }

// void loop(){
//   // Nothing to do here, FreeRTOS tasks handle the work
//   Serial.println("Hello World");

//   // Bật LED
//   digitalWrite(ledPin, HIGH);
//   delay(500);  // Giữ LED sáng 500ms

//   // Tắt LED
//   digitalWrite(ledPin, LOW);
//   delay(500);  // Giữ LED tắt 500ms
// }

//==================================// gửi nhiệt độ độ ẩm lên web
// #include <WiFi.h>
// #include <PubSubClient.h> // MQTT Client
// #include <Wire.h>
// #include "DHT20.h" // Thư viện DHT20

// // Wi-Fi Credentials
// const char *ssid = "viet";
// const char *password = "20252025";

// // ThingsBoard MQTT Broker
// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *ACCESS_TOKEN = "gB69jhkhOWD1wEYj6mm7"; // Thay bằng token thiết bị của bạn

// WiFiClient espClient;
// PubSubClient client(espClient);
// DHT20 dht20;
// float temperature = 30;
// float humidity = 50;
// // **Kết nối WiFi**
// void connectWiFi()
// {
//     Serial.print("Connecting to WiFi...");
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.print(".");
//         delay(1000);
//     }
//     Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
// }

// // **Kết nối MQTT đến ThingsBoard**
// void reconnectMQTT()
// {
//     while (!client.connected())
//     {
//         Serial.print("Connecting to MQTT...");
//         if (client.connect("ESP32_S3", ACCESS_TOKEN, ""))
//         {
//             Serial.println("Connected to ThingsBoard!");
//         }
//         else
//         {
//             Serial.print("Failed, rc=");
//             Serial.print(client.state());
//             Serial.println(" retrying in 5 seconds...");
//             delay(5000);
//         }
//     }
// }

// // **Gửi dữ liệu cảm biến lên ThingsBoard**
// void sendTelemetry()
// {
//     if (!dht20.read())
//     { // Đọc dữ liệu từ DHT20
//         float temperature = dht20.getTemperature();
//         float humidity = dht20.getHumidity();

//         // **Tạo JSON**
//         String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";

//         // **Kiểm tra kết nối trước khi gửi**
//         if (client.connected())
//         {
//             client.publish("v1/devices/me/telemetry", payload.c_str());
//             Serial.println("Sent telemetry: " + payload);
//         }
//         else
//         {
//             Serial.println("MQTT not connected, cannot send data!");
//         }
//     }
//     else
//     {
//         Serial.println("Failed to read DHT20 sensor!");
//     }
// }

// void setup()
// {
//     Serial.begin(115200);
//     Wire.begin();
//     dht20.begin();

//     connectWiFi();
//     client.setServer(mqttServer, mqttPort);
// }

// void loop()
// {
//     if (!client.connected())
//     {
//         reconnectMQTT();
//     }
//     client.loop();

//     sendTelemetry(); // Gửi dữ liệu cảm biến mỗi 5 giây
//     delay(3000);
// }

//=================================//
// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <Wire.h>
// #include "DHT20.h"
// #include <ArduinoJson.h>

// // Wi-Fi Credentials
// const char *ssid = "viet";
// const char *password = "20252025";

// // ThingsBoard MQTT Broker
// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *ACCESS_TOKEN = "gB69jhkhOWD1wEYj6mm7";

// WiFiClient espClient;
// PubSubClient client(espClient);
// DHT20 dht20;

// // **Khai báo chân LED**
// #define LED_PIN 13
// unsigned long lastTelemetryTime = 0; // Thời gian gửi dữ liệu trước đó
// const long telemetryInterval = 5000; // Gửi dữ liệu cảm biến mỗi 5 giây

// // **Kết nối WiFi**
// void connectWiFi()
// {
//     Serial.print("Connecting to WiFi...");
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.print(".");
//         delay(1000);
//     }
//     Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
// }

// // **Kết nối MQTT đến ThingsBoard**
// void reconnectMQTT()
// {
//     while (!client.connected())
//     {
//         Serial.print("Connecting to MQTT...");
//         if (client.connect("ESP32_S3", ACCESS_TOKEN, ""))
//         {
//             Serial.println("Connected to ThingsBoard!");
//             client.subscribe("v1/devices/me/rpc/request/+"); // Đăng ký nhận lệnh từ ThingsBoard
//         }
//         else
//         {
//             Serial.print("Failed, rc=");
//             Serial.print(client.state());
//             Serial.println(" retrying in 5 seconds...");
//             delay(5000);
//         }
//     }
// }

// // **Xử lý lệnh từ ThingsBoard để bật/tắt LED**
// void callback(char *topic, byte *payload, unsigned int length)
// {
//     Serial.print("Received MQTT Message: ");
//     payload[length] = '\0';
//     Serial.println((char *)payload);

//     // Parse JSON từ ThingsBoard
//     StaticJsonDocument<256> doc;
//     DeserializationError error = deserializeJson(doc, payload);

//     if (error)
//     {
//         Serial.print("JSON parse failed: ");
//         Serial.println(error.c_str());
//         return;
//     }

//     // Kiểm tra nếu nhận lệnh "setValue"
//     if (doc["method"] == "setValue")
//     {
//         bool ledState = doc["params"];
//         digitalWrite(LED_PIN, ledState ? HIGH : LOW);
//         Serial.println(ledState ? "LED ON" : "LED OFF");

//         // Phản hồi trạng thái LED về ThingsBoard
//         StaticJsonDocument<128> response;
//         response["value"] = ledState;
//         char buffer[128];
//         serializeJson(response, buffer);
//         client.publish("v1/devices/me/attributes", buffer);
//     }
// }

// // **Gửi dữ liệu cảm biến lên ThingsBoard (không delay)**
// void sendTelemetry()
// {
//     unsigned long currentMillis = millis();
//     if (currentMillis - lastTelemetryTime >= telemetryInterval)
//     {
//         lastTelemetryTime = currentMillis; // Cập nhật thời gian gửi dữ liệu

//         if (!dht20.read())
//         {
//             float temperature = dht20.getTemperature();
//             float humidity = dht20.getHumidity();

//             // **Tạo JSON**
//             StaticJsonDocument<128> doc;
//             doc["temperature"] = temperature;
//             doc["humidity"] = humidity;

//             char buffer[128];
//             serializeJson(doc, buffer);

//             // **Kiểm tra kết nối trước khi gửi**
//             if (client.connected())
//             {
//                 client.publish("v1/devices/me/telemetry", buffer);
//                 Serial.println("Sent telemetry: " + String(buffer));
//             }
//             else
//             {
//                 Serial.println("MQTT not connected, cannot send data!");
//             }
//         }
//         else
//         {
//             Serial.println("Failed to read DHT20 sensor!");
//         }
//     }
// }

// void setup()
// {
//     Serial.begin(115200);
//     Wire.begin();
//     dht20.begin();

//     pinMode(LED_PIN, OUTPUT);
//     digitalWrite(LED_PIN, LOW);

//     connectWiFi();
//     client.setServer(mqttServer, mqttPort);
//     client.setCallback(callback);
// }

// void loop()
// {
//     if (!client.connected())
//     {
//         reconnectMQTT();
//     }
//     client.loop(); // Xử lý MQTT ngay lập tức

//     sendTelemetry(); // Gửi dữ liệu cảm biến theo thời gian mà không delay
// }

//===================================

//=================================//

//============

// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>
// #include <Wire.h>

// // Wi-Fi Credentials
// const char *ssid = "viet";
// const char *password = "20252025";

// // ThingsBoard MQTT Broker
// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *ACCESS_TOKEN = "gB69jhkhOWD1wEYj6mm7"; // Token của thiết bị MQ2

// WiFiClient espClient;
// PubSubClient client(espClient);

// // Chân Analog MQ2 (AO)
// #define MQ2_AO_PIN 1

// unsigned long lastTelemetryTime = 0;
// const long telemetryInterval = 5000; // Gửi dữ liệu mỗi 5 giây

// // Task handles
// TaskHandle_t WiFiTaskHandle = NULL;
// TaskHandle_t MQTTaskHandle = NULL;
// TaskHandle_t MQ2TaskHandle = NULL;

// // **Kết nối WiFi** - Tạo một tác vụ riêng cho WiFi
// void WiFiTask(void *pvParameters)
// {
//     // Kết nối Wi-Fi một lần
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.print(".");
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 giây
//     }
//     Serial.println("\nWiFi connected: " + WiFi.localIP().toString());

//     // Kiểm tra kết nối Wi-Fi trong chu kỳ 10 giây
//     unsigned long lastCheck = millis();
//     while (true)
//     {
//         if (millis() - lastCheck >= 10000) // Kiểm tra kết nối Wi-Fi mỗi 10 giây
//         {
//             lastCheck = millis();
//             if (WiFi.status() != WL_CONNECTED)
//             {
//                 Serial.println("WiFi lost, reconnecting...");
//                 WiFi.reconnect();
//                 while (WiFi.status() != WL_CONNECTED)
//                 {
//                     vTaskDelay(pdMS_TO_TICKS(1000)); // Chờ trong khi kết nối lại
//                 }
//                 Serial.println("WiFi reconnected: " + WiFi.localIP().toString());
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Tiếp tục thực hiện công việc khác nếu không có sự thay đổi
//     }
// }

// // **Kết nối MQTT đến ThingsBoard** - Tạo một tác vụ riêng cho MQTT
// void MQTTask(void *pvParameters)
// {
//     // Kết nối MQTT một lần
//     while (!client.connected())
//     {
//         Serial.print("Connecting to MQTT...");
//         if (client.connect("ESP32_MQ2", ACCESS_TOKEN, ""))
//         {
//             Serial.println("Connected to ThingsBoard!");
//         }
//         else
//         {
//             Serial.print("Failed, rc=");
//             Serial.print(client.state());
//             Serial.println(" retrying in 5 seconds...");
//             vTaskDelay(pdMS_TO_TICKS(5000)); // Delay 5 giây
//         }
//     }

//     // Kiểm tra kết nối MQTT trong chu kỳ 10 giây
//     unsigned long lastCheck = millis();
//     while (true)
//     {
//         if (millis() - lastCheck >= 10000) // Kiểm tra kết nối MQTT mỗi 10 giây
//         {
//             lastCheck = millis();
//             if (!client.connected())
//             {
//                 Serial.println("MQTT disconnected, reconnecting...");
//                 while (!client.connected())
//                 {
//                     if (client.connect("ESP32_MQ2", ACCESS_TOKEN, ""))
//                     {
//                         Serial.println("Reconnected to ThingsBoard!");
//                     }
//                     else
//                     {
//                         Serial.print("Failed to reconnect, rc=");
//                         Serial.print(client.state());
//                         Serial.println(" retrying in 5 seconds...");
//                         vTaskDelay(pdMS_TO_TICKS(5000)); // Delay 5 giây nếu không kết nối lại được
//                     }
//                 }
//             }
//         }
//         client.loop();                   // Giữ MQTT hoạt động
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Kiểm tra mỗi giây
//     }
// }

// // **Gửi dữ liệu MQ2 lên ThingsBoard** - Tạo một tác vụ riêng để gửi dữ liệu MQ2
// void sendMQ2Data(void *pvParameters)
// {
//     while (1)
//     {
//         int gasAnalogValue = analogRead(MQ2_AO_PIN);
//         Serial.println(gasAnalogValue);

//         StaticJsonDocument<128> doc;
//         doc["mq2_analog"] = gasAnalogValue;

//         char buffer[128];
//         serializeJson(doc, buffer);

//         // Gửi dữ liệu nếu MQTT đã kết nối
//         if (client.connected())
//         {
//             client.publish("v1/devices/me/telemetry", buffer);
//             // Serial.println("Sent MQ2 data: " + String(buffer));
//         }
//         else
//         {
//             Serial.println("MQTT not connected, cannot send MQ2 data!");
//         }

//         vTaskDelay(pdMS_TO_TICKS(telemetryInterval)); // Gửi dữ liệu mỗi 5 giây
//     }
// }

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(MQ2_AO_PIN, INPUT);

//     // Create tasks
//     xTaskCreate(WiFiTask, "WiFiTask", 4096, NULL, 1, &WiFiTaskHandle);
//     xTaskCreate(MQTTask, "MQTTask", 4096, NULL, 1, &MQTTaskHandle);
//     xTaskCreate(sendMQ2Data, "SendMQ2Data", 2048, NULL, 1, &MQ2TaskHandle);
// }

// void loop()
// {
//     // FreeRTOS handles tasks, so no need for code here
// }
//======================//
// #include <WiFi.h>
// #include <Wire.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>
// #include "DHT20.h"

// const char *ssid = "viet";
// const char *password = "20252025";

// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *ACCESS_TOKEN = "gB69jhkhOWD1wEYj6mm7"; // Token của thiết bị

// WiFiClient espClient;
// PubSubClient client(espClient);

// // Chân Analog MQ2 (AO)
// #define MQ2_AO_PIN 1
// DHT20 dht20;

// unsigned long lastTelemetryTime = 0;
// unsigned long lastMQ2Time = 0;
// const long telemetryInterval = 5000; // Gửi dữ liệu mỗi 5 giây
// const long MQ2Interval = 5000;       // Gửi dữ liệu MQ2 mỗi 5 giây

// // Kết nối WiFi
// void connectWiFi()
// {
//     Serial.print("Connecting to WiFi...");
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.print(".");
//         delay(1000);
//     }
//     Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
// }

// // Kết nối lại MQTT
// void reconnectMQTT()
// {
//     while (!client.connected())
//     {
//         Serial.print("Connecting to MQTT...");
//         if (client.connect("ESP32_MQ2", ACCESS_TOKEN, ""))
//         {
//             Serial.println("Connected to ThingsBoard!");
//         }
//         else
//         {
//             Serial.print("Failed, rc=");
//             Serial.print(client.state());
//             Serial.println(" retrying in 5 seconds...");
//             delay(5000);
//         }
//     }
// }

// // Gửi dữ liệu MQ2
// void sendMQ2Data()
// {
//     unsigned long currentMillis = millis();
//     if (currentMillis - lastMQ2Time >= MQ2Interval)
//     {
//         lastMQ2Time = currentMillis;

//         int gasAnalogValue = analogRead(MQ2_AO_PIN);
//         Serial.println(gasAnalogValue);

//         StaticJsonDocument<128> doc;
//         doc["mq2_analog"] = gasAnalogValue;

//         char buffer[128];
//         serializeJson(doc, buffer);

//         if (client.connected())
//         {
//             client.publish("v1/devices/me/telemetry", buffer);
//             Serial.println("Sent MQ2 data: " + String(buffer));
//         }
//         else
//         {
//             Serial.println("MQTT not connected, cannot send MQ2 data!");
//         }
//     }
// }
// float temperature = 30;
// float humidity = 50;
// // Gửi telemetry từ DHT20
// void sendTelemetry()
// {
//     dht20.begin();
//     unsigned long currentMillis = millis();
//     if (currentMillis - lastTelemetryTime >= telemetryInterval)
//     {
//         lastTelemetryTime = currentMillis;

//         if (dht20.read())
//         {
//             // float temperature = dht20.getTemperature();
//             // float humidity = dht20.getHumidity();

//             Serial.println(temperature);
//             Serial.println(humidity);
//             StaticJsonDocument<128> doc;
//             doc["temperature"] = temperature;
//             doc["humidity"] = humidity;

//             char buffer[128];
//             serializeJson(doc, buffer);

//             if (client.connected())
//             {
//                 client.publish("v1/devices/me/telemetry", buffer);
//                 Serial.println("Sent telemetry: " + String(buffer));
//             }
//             else
//             {
//                 Serial.println("MQTT not connected, cannot send telemetry data!");
//             }
//         }
//         else
//         {
//             Serial.println("Failed to read DHT20 sensor!");
//         }
//         temperature++;
//         humidity++;
//     }
// }

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(MQ2_AO_PIN, INPUT);

//     connectWiFi();
//     client.setServer(mqttServer, mqttPort);
// }

// void loop()
// {
//     if (!client.connected())
//     {
//         reconnectMQTT();
//     }
//     client.loop();

//     sendMQ2Data();
//     sendTelemetry();
// }
//======================//
//==============//

#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "DHT20.h"

// WiFi Credentials
const char *ssid = "viet";
const char *password = "20252025";

// ThingsBoard MQTT Broker
const char *mqttServer = "app.coreiot.io";
const int mqttPort = 1883;
const char *ACCESS_TOKEN = "gB69jhkhOWD1wEYj6mm7"; // Token của thiết bị

WiFiClient espClient;
PubSubClient client(espClient);

// Chân Analog MQ2 (AO)
#define MQ2_AO_PIN 1
DHT20 dht20;
#define WIFI_TIMEOUT_MS 20000
unsigned long lastTelemetryTime = 0;
unsigned long lastMQ2Time = 0;
const long telemetryInterval = 5000; // Gửi dữ liệu mỗi 5 giây
const long MQ2Interval = 5000;       // Gửi dữ liệu MQ2 mỗi 5 giây

// Task handles
TaskHandle_t WiFiTaskHandle = NULL;
TaskHandle_t MQTTaskHandle = NULL;
TaskHandle_t MQ2TaskHandle = NULL;
TaskHandle_t TelemetryTaskHandle = NULL;

// Kết nối WiFi
void connectWiFi(void *pvParameters)
{
    for (;;)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("wifi still connected");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        Serial.print("Connecting to WiFi...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        unsigned long start = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS)
        {
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Wifi failed");
            vTaskDelay(20000 / portTICK_PERIOD_MS);
            continue;
        }

        Serial.println("\nWiFi connected: " + WiFi.localIP());
    }
}

// Kết nối lại MQTT
void reconnectMQTT()
{
    while (!client.connected())
    {
        Serial.print("Connecting to MQTT...");
        if (client.connect("ESP32", ACCESS_TOKEN, ""))
        {
            Serial.println("Connected to ThingsBoard!");
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds...");
            vTaskDelay(pdMS_TO_TICKS(5000)); // Delay 5 giây
        }
    }
}

// Gửi dữ liệu MQ2
// Task gửi dữ liệu MQ2
void sendMQ2Data(void *pvParameters)
{
    while (1)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastMQ2Time >= MQ2Interval)
        {
            lastMQ2Time = currentMillis;

            int gasAnalogValue = analogRead(MQ2_AO_PIN);
            Serial.println(gasAnalogValue);

            StaticJsonDocument<128> doc;
            doc["mq2_analog"] = gasAnalogValue;

            char buffer[128];
            serializeJson(doc, buffer);

            if (client.connected()) // Kiểm tra kết nối MQTT
            {
                client.publish("v1/devices/me/telemetry", buffer);
                Serial.println("Sent MQ2 data: " + String(buffer));
            }
            else
            {
                Serial.println("MQTT not connected, waiting for reconnection...");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000)); // Gửi dữ liệu MQ2 mỗi 5 giây
    }
}

// Task gửi telemetry từ DHT20
void sendTelemetry(void *pvParameters)
{
    dht20.begin();
    while (1)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastTelemetryTime >= telemetryInterval)
        {
            lastTelemetryTime = currentMillis;

            if (dht20.read())
            {
                // Đọc dữ liệu nhiệt độ và độ ẩm từ cảm biến
                float temperature = dht20.getTemperature();
                float humidity = dht20.getHumidity();

                Serial.println(temperature);
                Serial.println(humidity);

                StaticJsonDocument<128> doc;
                doc["temperature"] = temperature;
                doc["humidity"] = humidity;

                char buffer[128];
                serializeJson(doc, buffer);

                if (client.connected()) // Kiểm tra kết nối MQTT
                {
                    client.publish("v1/devices/me/telemetry", buffer);
                    Serial.println("Sent telemetry: " + String(buffer));
                }
                else
                {
                    Serial.println("MQTT not connected, waiting for reconnection...");
                }
            }
            else
            {
                Serial.println("Failed to read DHT20 sensor!");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(telemetryInterval)); // Gửi dữ liệu từ DHT20 mỗi 5 giây
    }
}

// Task MQTT
void MQTTask(void *pvParameters)
{
    // Tạo kết nối MQTT
    reconnectMQTT();

    // Kiểm tra kết nối liên tục trong task riêng biệt
    while (1)
    {
        if (!client.connected())
        {
            reconnectMQTT(); // Kết nối lại MQTT nếu mất kết nối
        }
        client.loop();                        // Đảm bảo client MQTT nhận và gửi dữ liệu
        vTaskDelay(500 / portTICK_PERIOD_MS); // Chạy mỗi 1 giây
    }
}

// Tạo MQTT task

void setup()
{
    Serial.begin(115200);
    client.setServer(mqttServer, mqttPort);
    pinMode(MQ2_AO_PIN, INPUT);

    // Create tasks
    xTaskCreate(connectWiFi, "WiFiTask", 5000, NULL, 1, &WiFiTaskHandle);
    xTaskCreate(MQTTask, "MQTTask", 4096, NULL, 1, &MQTTaskHandle);
    xTaskCreate(sendMQ2Data, "MQ2Task", 2048, NULL, 1, &MQ2TaskHandle);
    xTaskCreate(sendTelemetry, "TelemetryTask", 2048, NULL, 1, &TelemetryTaskHandle);
}

void loop()
{
    // FreeRTOS will handle tasks, no need for code here
}
