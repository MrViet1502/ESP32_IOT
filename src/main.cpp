// Code freeRTOS connect WiFi, MQTT và gửi dữ liệu lên ThingsBoard

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
const char *ACCESS_TOKEN = "gB69jhkhOWD1wEYj6mm7";

WiFiClient espClient;
PubSubClient client(espClient);

// Cảm biến
DHT20 dht20;
#define MQ2_AO_PIN 34 //

// Thời gian gửi
const long telemetryInterval = 5000;
const long mq2Interval = 5000;
SemaphoreHandle_t i2cMutex;
// Task handles
TaskHandle_t WiFiTaskHandle = NULL;
TaskHandle_t MQTTaskHandle = NULL;
TaskHandle_t MQ2TaskHandle = NULL;
TaskHandle_t TelemetryTaskHandle = NULL;

//  WiFi Task: Kiểm tra reconnect nếu rớt mạng
void checkWiFiTask(void *pvParameters)
{
    for (;;)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println(" WiFi disconnected! Reconnecting...");
            WiFi.begin(ssid, password);

            unsigned long start = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
            {
                delay(500);
                Serial.print(".");
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.print("\n WiFi connected: ");
                Serial.println(WiFi.localIP());
            }
            else
            {
                Serial.println("\n WiFi reconnect failed.");
            }
        }
        else
        {
            Serial.println(" WiFi OK");
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS); // Kiểm tra mỗi 10 giây
    }
}

//  MQTT Task
void reconnectMQTT()
{
    while (!client.connected())
    {
        Serial.print(" Connecting to MQTT...");
        if (client.connect("ESP32", ACCESS_TOKEN, ""))
        {
            Serial.println(" MQTT Connected to ThingsBoard");
        }
        else
        {
            Serial.print(" Failed, rc=");
            Serial.print(client.state());
            Serial.println(" → retry in 5s");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

void MQTTask(void *pvParameters)
{
    for (;;)
    {
        if (!client.connected())
        {
            reconnectMQTT();
        }
        client.loop();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

// 🌡 Task gửi DHT20 telemetry
void sendTelemetry(void *pvParameters)
{
    dht20.begin();

    for (;;)
    {
        if (WiFi.status() == WL_CONNECTED && client.connected())
        {
            //  Lấy quyền truy cập I2C
            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                bool ok = dht20.read();
                if (!ok)
                {
                    float temp = dht20.getTemperature();
                    float hum = dht20.getHumidity();

                    StaticJsonDocument<128> doc;
                    doc["temperature"] = temp;
                    doc["humidity"] = hum;

                    char buffer[128];
                    serializeJson(doc, buffer);

                    client.publish("v1/devices/me/telemetry", buffer);
                    Serial.println(" Sent DHT20: " + String(buffer));
                }
                else
                {
                    Serial.println(" DHT20 read failed");
                }

                //  Trả quyền lại
                xSemaphoreGive(i2cMutex);
            }
            else
            {
                Serial.println(" I2C busy, skip DHT20 read");
            }
        }

        vTaskDelay(telemetryInterval / portTICK_PERIOD_MS);
    }
}

//  Task gửi MQ2 telemetry
void sendMQ2Data(void *pvParameters)
{
    for (;;)
    {
        if (WiFi.status() == WL_CONNECTED && client.connected())
        {
            int mq2Value = analogRead(MQ2_AO_PIN);
            StaticJsonDocument<128> doc;
            doc["mq2_analog"] = mq2Value;

            char buffer[128];
            serializeJson(doc, buffer);
            client.publish("v1/devices/me/telemetry", buffer);

            Serial.println(" Sent MQ2: " + String(buffer));
        }

        vTaskDelay(mq2Interval / portTICK_PERIOD_MS);
    }
}

//  setup()
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Wire.begin();
    i2cMutex = xSemaphoreCreateMutex();
    pinMode(MQ2_AO_PIN, INPUT);
    WiFi.begin(ssid, password);
    client.setServer(mqttServer, mqttPort);

    //  Tạo task FreeRTOS
    xTaskCreate(checkWiFiTask, "WiFiTask", 4096, NULL, 1, &WiFiTaskHandle);
    xTaskCreate(MQTTask, "MQTTask", 4096, NULL, 1, &MQTTaskHandle);
    xTaskCreate(sendTelemetry, "TelemetryTask", 4096, NULL, 1, &TelemetryTaskHandle);
    xTaskCreate(sendMQ2Data, "MQ2Task", 4096, NULL, 1, &MQ2TaskHandle);
}

// loop bỏ trống
void loop() {}
//=================================//

//=================================//
// Code sent data from sensor to coreIOT
// #include "WiFi.h"
// #include "ESPAsyncWebServer.h"
// #include "SPIFFS.h"
// #include "DHT20.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include <Arduino.h>
// #include <PubSubClient.h> // Thư viện MQTT cho ESP32

// // Replace with your network credentials

// const char *ssid = "viet";
// const char *password = "20252025";

// // ThingsBoard Credentials
// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *ACCESS_TOKEN = "viet"; // Thay bằng token thiết bị trên ThingsBoard

// WiFiClient espClient;
// PubSubClient client(espClient);
// DHT20 dht20;

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

// void reconnectMQTT()
// {
//     while (!client.connected())
//     {
//         Serial.print("Connecting to MQTT...");
//         if (client.connect("ESP32", ACCESS_TOKEN, ""))
//         {
//             Serial.println("Connected to MQTT!");
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

// void sendTelemetry()
// {
//     if (!dht20.read())
//     { // Đọc dữ liệu từ DHT20
//         float temperature = dht20.getTemperature();
//         float humidity = dht20.getHumidity();

//         // Tạo JSON để gửi dữ liệu lên ThingsBoard
//         String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
//         client.publish("v1/devices/me/telemetry", payload.c_str());

//         Serial.println("Sent telemetry: " + payload);
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
//     delay(5000);
// }

//=================================//
// Bat tat LED va gui data len coreIOT thanh cong
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
//     digitalWrite(LED_PIN, HIGH);

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
//     client.loop();   // Xử lý MQTT ngay lập tức
//     sendTelemetry(); // Gửi dữ liệu cảm biến theo thời gian mà không delay
// }

//===================================

//=================================//
// Test freertos với MQTT và gửi dữ liệu lên ThingsBoard
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

//======================================
