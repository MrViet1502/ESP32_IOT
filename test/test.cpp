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

// =================================================== Update OTA control led
// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>

// #define LED_PIN 12
// #define LED_STATE_ATTR "led"

// const char *ssid = "viet";
// const char *password = "20252025";

// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *mqttUsername = "gB69jhkhOWD1wEYj6mm7";
// const char *mqttPassword = "";

// WiFiClient wifiClient;
// PubSubClient mqttClient(wifiClient);

// void connectToWiFi()
// {
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.println("Connecting to WiFi...");
//         delay(1000);
//     }
//     Serial.println("Connected to WiFi");
// }

// void connectToMQTT()
// {
//     mqttClient.setServer(mqttServer, mqttPort);
//     while (!mqttClient.connected())
//     {
//         Serial.println("Connecting to MQTT...");
//         if (mqttClient.connect("ESP32Client", mqttUsername, mqttPassword))
//         {
//             Serial.println("Connected to MQTT!");
//             mqttClient.subscribe("v1/devices/me/attributes");
//             mqttClient.subscribe("v1/devices/me/rpc/request/+");
//         }
//         else
//         {
//             Serial.print("Failed, rc=");
//             Serial.print(mqttClient.state());
//             Serial.println(" Retrying in 5 seconds...");
//             delay(5000);
//         }
//     }
// }

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

//     // Xử lý Shared Attribute update
//     if (topicStr.startsWith("v1/devices/me/attributes"))
//     {
//         if (doc.containsKey(LED_STATE_ATTR))
//         {
//             bool ledState = doc[LED_STATE_ATTR];
//             digitalWrite(LED_PIN, ledState ? HIGH : LOW);
//             Serial.print("LED state set from Shared Attribute: ");
//             Serial.println(ledState);
//         }
//     }

//     // Xử lý RPC
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
//             mqttClient.publish("v1/devices/me/attributes", buffer);
//         }
//     }
// }

// void setup()
// {
//     Serial.begin(115200);
//     pinMode(LED_PIN, OUTPUT);
//     connectToWiFi();
//     mqttClient.setCallback(callback);
//     connectToMQTT();
// }

// void loop()
// {
//     if (!mqttClient.connected())
//     {
//         connectToMQTT();
//     }
//     mqttClient.loop();
// }

// #include "certs.h"
// #include <WiFi.h>
// #include <PubSubClient.h>
// #include "esp_https_ota.h"
// #include "esp_log.h"

// // ====== WiFi & MQTT Config ======
// const char *ssid = "viet";
// const char *password = "20252025";

// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *mqttUsername = "gB69jhkhOWD1wEYj6mm7"; // Access token của thiết bị
// const char *mqttPassword = "";
// const char *otaTopic = "v1/devices/me/attributes";

// // ====== Global Variables ======
// WiFiClient wifiClient;
// PubSubClient mqttClient(wifiClient);
// String firmwareURL = "";
// bool otaTriggered = false;

// // ====== WiFi Connect ======
// void connectToWiFi()
// {
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.println("Connecting to WiFi...");
//         delay(1000);
//     }
//     Serial.println("Connected to WiFi!");
// }

// // ====== MQTT Callback ======
// void mqttCallback(char *topic, byte *payload, unsigned int length)
// {
//     Serial.printf("Received on topic: %s\n", topic);
//     String message;
//     for (int i = 0; i < length; i++)
//     {
//         message += (char)payload[i];
//     }
//     Serial.printf("Payload: %s\n", message.c_str());

//     // Extract URL from JSON
//     int urlIndex = message.indexOf("fw_url");
//     if (urlIndex != -1)
//     {
//         int start = message.indexOf("http", urlIndex);
//         int end = message.indexOf("\"", start);
//         firmwareURL = message.substring(start, end);
//         Serial.println("Received OTA URL: " + firmwareURL);
//         otaTriggered = true;
//     }
// }

// // ====== MQTT Connect ======
// void connectToMQTT()
// {
//     mqttClient.setServer(mqttServer, mqttPort);
//     while (!mqttClient.connected())
//     {
//         Serial.println("Connecting to MQTT...");
//         if (mqttClient.connect("ESP32Client", mqttUsername, mqttPassword))
//         {
//             Serial.println("Connected to MQTT!");
//             mqttClient.subscribe(otaTopic);
//         }
//         else
//         {
//             Serial.print("Failed, rc=");
//             Serial.print(mqttClient.state());
//             Serial.println(" Retrying in 5 seconds...");
//             delay(5000);
//         }
//     }
// }

// // ====== OTA Update Task ======
// void otaUpdateTask(void *param)
// {
//     for (;;)
//     {
//         if (otaTriggered && firmwareURL.length() > 0)
//         {
//             Serial.println("[OTA] Starting update from: " + firmwareURL);

//             esp_http_client_config_t config = {
//                 .url = firmwareURL.c_str(),
//                 .cert_pem = NULL,
//                 .timeout_ms = 15000,
//                 .skip_cert_common_name_check = true,
//             };

//             esp_err_t ret = esp_https_ota(&config);
//             if (ret == ESP_OK)
//             {
//                 Serial.println("[OTA] Update successful! Rebooting...");
//                 delay(2000);
//                 esp_restart();
//             }
//             else
//             {
//                 Serial.printf("[OTA] Update failed! Error: %s\n", esp_err_to_name(ret));
//             }
//             otaTriggered = false;
//         }
//         delay(5000);
//     }
// }

// // ====== Setup ======
// void setup()
// {
//     Serial.begin(115200);
//     connectToWiFi();
//     mqttClient.setCallback(mqttCallback);
//     connectToMQTT();

//     // Start OTA task
//     xTaskCreatePinnedToCore(otaUpdateTask, "ota_task", 8192, NULL, 1, NULL, 1);
// }

// // ====== Loop ======
// void loop()
// {
//     if (!mqttClient.connected())
//     {
//         connectToMQTT();
//     }
//     mqttClient.loop();
// }