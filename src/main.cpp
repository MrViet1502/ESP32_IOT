
// Code FreeRTOS Controller LED & DHT20 + MQ2 sensor send to CoreIOT and OTA update
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "DHT20.h"
#include <HTTPClient.h>       //
#include <WiFiClientSecure.h> //
#include <ArduinoOTA.h>

// WiFi Credentials
const char *ssid = "viet";
const char *password = "20252025";
#define LED_PIN 12
#define LED_STATE_ATTR "led" // Tên shared attribute để điều khiển LED
#define OTA_Key "fw_url"     // Tên key trong JSON để nhận URL firmware
// ThingsBoard MQTT Broker
const char *mqttServer = "app.coreiot.io";
const int mqttPort = 1883;
const char *ACCESS_TOKEN = "gB69jhkhOWD1wEYj6mm7";

WiFiClient espClient;
PubSubClient client(espClient);
String firmwareURL = "";             // URL cập nhật firmware
bool otaTriggered = false;           //
volatile bool otaInProgress = false; //
// Cảm biến
DHT20 dht20;
#define MQ2_AO_PIN 34 // Analog input cho MQ2

const long telemetryInterval = 5000;
const long mq2Interval = 5000;
SemaphoreHandle_t i2cMutex;

// Task handles
TaskHandle_t WiFiTaskHandle = NULL;
TaskHandle_t MQTTaskHandle = NULL;
TaskHandle_t MQ2TaskHandle = NULL;
TaskHandle_t TelemetryTaskHandle = NULL;
TaskHandle_t OTAUpdateTaskHandle = NULL;

bool OTA_Update(String url)
{
    HTTPClient http;
    http.begin(url);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();

    if (httpCode == 200)
    {
        int len = http.getSize();
        WiFiClient *stream = http.getStreamPtr();

        if (!Update.begin(len))
        {
            Serial.println("Update Begin Failed");
            return false;
        }

        size_t written = Update.writeStream(*stream);
        if (written == len)
        {
            Serial.println("Written : " + String(written) + " successfully");
        }
        else
        {
            Serial.println("Written only : " + String(written) + "/" + String(len) + ". Retry?");
            return false;
        }

        if (Update.end())
        {
            if (Update.isFinished())
            {
                Serial.println("Update successfully completed. Rebooting.");
                return true;
            }
            else
            {
                Serial.println("Update not finished? Something went wrong!");
                return false;
            }
        }
        else
        {
            Serial.println("Update.end() failed: " + String(Update.getError()));
            return false;
        }
    }
    else
    {
        Serial.printf("HTTP GET failed. Code: %d\n", httpCode);
        return false;
    }
}
void RunOTA_Update(void *pvParameters)
{
    for (;;)
    {
        if (otaTriggered && firmwareURL.length() > 0)
        {
            otaInProgress = true;
            Serial.println("[OTA] Starting update from: " + firmwareURL);
            if (OTA_Update(firmwareURL))
            {
                Serial.println("[OTA] Update successful! Rebooting...");
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                esp_restart();
            }
            else
            {
                Serial.println("[OTA] Update failed!");
            }
            otaTriggered = false;
            otaInProgress = false;
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    payload[length] = '\0';
    String topicStr = String(topic);
    Serial.print("Received on topic: ");
    Serial.println(topicStr);
    Serial.print("Payload: ");
    Serial.println((char *)payload);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Xử lý Shared Attribute update
    if (topicStr.startsWith("v1/devices/me/attributes"))
    {
        if (doc.containsKey(LED_STATE_ATTR))
        {
            bool ledState = doc[LED_STATE_ATTR];
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            Serial.print("LED state set from Shared Attribute: ");
            Serial.println(ledState);
        }
        if (doc.containsKey(OTA_Key))
        {
            firmwareURL = doc["fw_url"].as<String>();
            otaTriggered = true;
            Serial.println("Received OTA URL: " + firmwareURL);
        }
    }

    // Xử lý RPC
    if (topicStr.startsWith("v1/devices/me/rpc/request"))
    {
        if (doc["method"] == "setValue")
        {
            bool ledState = doc["params"];
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            Serial.println(ledState ? "LED ON via RPC" : "LED OFF via RPC");

            StaticJsonDocument<128> response;
            response["value"] = ledState;
            char buffer[128];
            serializeJson(response, buffer);
            client.publish("v1/devices/me/attributes", buffer);
        }
    }
}

void checkWiFiTask(void *pvParameters)
{
    for (;;)
    {
        if (otaInProgress) // Nếu đang cập nhật OTA thì không kiểm tra WiFi
        {
            Serial.println("OTA rebooting ...");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            continue;
        }
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
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void reconnectMQTT()
{
    while (!client.connected())
    {
        Serial.print(" Connecting to MQTT...");
        if (client.connect("ESP32", ACCESS_TOKEN, ""))
        {
            Serial.println(" MQTT Connected to ThingsBoard");
            client.subscribe("v1/devices/me/rpc/request/+");
            client.subscribe("v1/devices/me/attributes"); // Lắng nghe Shared Attribute
            client.setCallback(callback);
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

void sendTelemetry(void *pvParameters)
{
    dht20.begin();
    for (;;)
    {
        if (otaInProgress) // Nếu đang cập nhật OTA thì không gửi telemetry
        {
            // Serial.println("OTA rebooting ...");
            vTaskDelay(telemetryInterval / portTICK_PERIOD_MS);
            continue;
        }
        if (WiFi.status() == WL_CONNECTED && client.connected())
        {
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

void sendMQ2Data(void *pvParameters)
{
    for (;;)
    {
        if (otaInProgress) // Nếu đang cập nhật OTA thì không gửi dữ liệu MQ2
        {
            // Serial.println("OTA rebooting ...");
            vTaskDelay(mq2Interval / portTICK_PERIOD_MS);
            continue;
        }
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

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Wire.begin();
    i2cMutex = xSemaphoreCreateMutex();
    pinMode(MQ2_AO_PIN, INPUT);
    WiFi.begin(ssid, password);
    client.setServer(mqttServer, mqttPort);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    client.setCallback(callback);
    xTaskCreate(checkWiFiTask, "WiFiTask", 4096, NULL, 1, &WiFiTaskHandle);
    xTaskCreate(MQTTask, "MQTTask", 4096, NULL, 1, &MQTTaskHandle);
    xTaskCreate(sendTelemetry, "TelemetryTask", 4096, NULL, 1, &TelemetryTaskHandle);
    xTaskCreate(sendMQ2Data, "MQ2Task", 4096, NULL, 1, &MQ2TaskHandle);
    xTaskCreate(RunOTA_Update, "OTA_Update", 8192, NULL, 1, &OTAUpdateTaskHandle); // Tạo task OTA
}

void loop() {}

//===================================//
//===================================//

// digitalWrite(LED_PIN, HIGH);
//             vTaskDelay(100 / portTICK_PERIOD_MS); // sáng 100ms
//             digitalWrite(LED_PIN, LOW);

//===================================//
//===================================//
// #include <WiFi.h>
// #include <Wire.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include "DHT20.h"
// #include "esp_https_ota.h"
// #include "esp_http_client.h"
// #include "certs.h"
// // WiFi Credentials
// const char *ssid = "viet";
// const char *password = "20252025";
// #define LED_PIN 12
// #define LED_STATE_ATTR "led"

// // ThingsBoard MQTT Broker
// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *ACCESS_TOKEN = "gB69jhkhOWD1wEYj6mm7";

// WiFiClient espClient;
// PubSubClient client(espClient);

// // Cảm biến
// DHT20 dht20;
// #define MQ2_AO_PIN 34
// const long telemetryInterval = 5000;
// const long mq2Interval = 5000;
// SemaphoreHandle_t i2cMutex;

// // Task handles
// TaskHandle_t WiFiTaskHandle = NULL;
// TaskHandle_t MQTTaskHandle = NULL;
// TaskHandle_t MQ2TaskHandle = NULL;
// TaskHandle_t TelemetryTaskHandle = NULL;
// TaskHandle_t OTATaskHandle = NULL;

// // OTA Variables
// String firmwareURL = "";
// bool otaTriggered = false;

// void callback(char *topic, byte *payload, unsigned int length)
// {
//     payload[length] = '\0';
//     String topicStr = String(topic);
//     Serial.print("Received on topic: ");
//     Serial.println(topicStr);
//     Serial.print("Payload: ");
//     Serial.println((char *)payload);

//     StaticJsonDocument<256> doc;
//     DeserializationError error = deserializeJson(doc, payload);
//     if (error)
//     {
//         Serial.print("JSON parse failed: ");
//         Serial.println(error.c_str());
//         return;
//     }

//     if (topicStr.startsWith("v1/devices/me/attributes"))
//     {
//         if (doc.containsKey(LED_STATE_ATTR))
//         {
//             bool ledState = doc[LED_STATE_ATTR];
//             digitalWrite(LED_PIN, ledState ? HIGH : LOW);
//             Serial.print("LED state set from Shared Attribute: ");
//             Serial.println(ledState);
//         }

//         if (doc.containsKey("fw_url"))
//         {
//             firmwareURL = doc["fw_url"].as<String>();
//             otaTriggered = true;
//             Serial.println("Received OTA URL: " + firmwareURL);
//         }
//     }

//     if (topicStr.startsWith("v1/devices/me/rpc/request"))
//     {
//         if (doc["method"] == "setValue")
//         {
//             bool ledState = doc["params"];
//             digitalWrite(LED_PIN, ledState ? HIGH : LOW);
//             Serial.println(ledState ? "LED ON via RPC" : "LED OFF via RPC");

//             StaticJsonDocument<128> response;
//             response["value"] = ledState;
//             char buffer[128];
//             serializeJson(response, buffer);
//             client.publish("v1/devices/me/attributes", buffer);
//         }
//     }
// }

// void otaUpdateTask(void *pvParameters)
// {
//     for (;;)
//     {
//         if (otaTriggered && firmwareURL.length() > 0)
//         {
//             Serial.println("[OTA] Starting update from: " + firmwareURL);

//             esp_http_client_config_t config = {};
//             config.url = firmwareURL.c_str();
//             config.cert_pem = github_root_cert; // ✅ Dùng CA cert lấy từ github_cert.pem
//             config.timeout_ms = 10000;

//             esp_err_t ret = esp_https_ota(&config);
//             if (ret == ESP_OK)
//             {
//                 Serial.println("[OTA] Update successful! Rebooting...");
//                 vTaskDelay(2000 / portTICK_PERIOD_MS);
//                 esp_restart();
//             }
//             else
//             {
//                 Serial.printf("[OTA] Update failed! Error: %s\n", esp_err_to_name(ret));
//             }
//             otaTriggered = false;
//         }
//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//     }
// }

// void checkWiFiTask(void *pvParameters)
// {
//     for (;;)
//     {
//         if (WiFi.status() != WL_CONNECTED)
//         {
//             Serial.println(" WiFi disconnected! Reconnecting...");
//             WiFi.begin(ssid, password);
//             unsigned long start = millis();
//             while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
//             {
//                 delay(500);
//                 Serial.print(".");
//             }
//             if (WiFi.status() == WL_CONNECTED)
//             {
//                 Serial.print("\n WiFi connected: ");
//                 Serial.println(WiFi.localIP());
//             }
//             else
//             {
//                 Serial.println("\n WiFi reconnect failed.");
//             }
//         }
//         else
//         {
//             Serial.println(" WiFi OK");
//         }
//         vTaskDelay(10000 / portTICK_PERIOD_MS);
//     }
// }

// void reconnectMQTT()
// {
//     while (!client.connected())
//     {
//         Serial.print(" Connecting to MQTT...");
//         if (client.connect("ESP32", ACCESS_TOKEN, ""))
//         {
//             Serial.println(" MQTT Connected to ThingsBoard");
//             client.subscribe("v1/devices/me/rpc/request/+");
//             client.subscribe("v1/devices/me/attributes");
//             client.setCallback(callback);
//         }
//         else
//         {
//             Serial.print(" Failed, rc=");
//             Serial.print(client.state());
//             Serial.println(" → retry in 5s");
//             vTaskDelay(5000 / portTICK_PERIOD_MS);
//         }
//     }
// }

// void MQTTask(void *pvParameters)
// {
//     for (;;)
//     {
//         if (!client.connected())
//         {
//             reconnectMQTT();
//         }
//         client.loop();
//         vTaskDelay(500 / portTICK_PERIOD_MS);
//     }
// }

// void sendTelemetry(void *pvParameters)
// {
//     dht20.begin();
//     for (;;)
//     {
//         if (WiFi.status() == WL_CONNECTED && client.connected())
//         {
//             if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE)
//             {
//                 bool ok = dht20.read();
//                 if (!ok)
//                 {
//                     float temp = dht20.getTemperature();
//                     float hum = dht20.getHumidity();
//                     StaticJsonDocument<128> doc;
//                     doc["temperature"] = temp;
//                     doc["humidity"] = hum;
//                     char buffer[128];
//                     serializeJson(doc, buffer);
//                     client.publish("v1/devices/me/telemetry", buffer);
//                     Serial.println(" Sent DHT20: " + String(buffer));
//                 }
//                 else
//                 {
//                     Serial.println(" DHT20 read failed");
//                 }
//                 xSemaphoreGive(i2cMutex);
//             }
//             else
//             {
//                 Serial.println(" I2C busy, skip DHT20 read");
//             }
//         }
//         vTaskDelay(telemetryInterval / portTICK_PERIOD_MS);
//     }
// }

// void sendMQ2Data(void *pvParameters)
// {
//     for (;;)
//     {
//         if (WiFi.status() == WL_CONNECTED && client.connected())
//         {
//             int mq2Value = analogRead(MQ2_AO_PIN);
//             StaticJsonDocument<128> doc;
//             doc["mq2_analog"] = mq2Value;
//             char buffer[128];
//             serializeJson(doc, buffer);
//             client.publish("v1/devices/me/telemetry", buffer);
//             Serial.println(" Sent MQ2: " + String(buffer));
//         }
//         vTaskDelay(mq2Interval / portTICK_PERIOD_MS);
//     }
// }

// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);
//     Wire.begin();
//     i2cMutex = xSemaphoreCreateMutex();
//     pinMode(MQ2_AO_PIN, INPUT);
//     WiFi.begin(ssid, password);
//     client.setServer(mqttServer, mqttPort);
//     pinMode(LED_PIN, OUTPUT);
//     digitalWrite(LED_PIN, LOW);
//     client.setCallback(callback);

//     xTaskCreate(checkWiFiTask, "WiFiTask", 4096, NULL, 1, &WiFiTaskHandle);
//     xTaskCreate(MQTTask, "MQTTask", 4096, NULL, 1, &MQTTaskHandle);
//     xTaskCreate(sendTelemetry, "TelemetryTask", 4096, NULL, 1, &TelemetryTaskHandle);
//     xTaskCreate(sendMQ2Data, "MQ2Task", 4096, NULL, 1, &MQ2TaskHandle);
//     xTaskCreate(otaUpdateTask, "OTATask", 8192, NULL, 1, &OTATaskHandle);
// }

// void loop() {}
