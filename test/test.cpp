
// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <HTTPClient.h>       // 👈 Thêm dòng này
// #include <WiFiClientSecure.h> // 👈 Nếu dùng HTTPS
// #include <ArduinoOTA.h>

// // WiFi
// const char *ssid = "viet";
// const char *password = "20252025";

// // MQTT CoreIoT (ThingsBoard)
// const char *mqttServer = "app.coreiot.io";
// const int mqttPort = 1883;
// const char *mqttUsername = "gB69jhkhOWD1wEYj6mm7";
// const char *mqttPassword = "";
// const char *otaTopic = "v1/devices/me/attributes";

// WiFiClient wifiClient;
// PubSubClient mqttClient(wifiClient);
// String firmwareURL = "";
// bool otaTriggered = false;

// // WiFi Connect
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

// // MQTT Connect
// void connectToMQTT()
// {
//     mqttClient.setServer(mqttServer, mqttPort);
//     while (!mqttClient.connected())
//     {
//         Serial.println("Connecting to MQTT...");
//         if (mqttClient.connect("ESP32Client", mqttUsername, mqttPassword))
//         {
//             Serial.println("Connected to MQTT");
//             mqttClient.subscribe(otaTopic);
//         }
//         else
//         {
//             Serial.print("MQTT failed, rc=");
//             Serial.println(mqttClient.state());
//             delay(5000);
//         }
//     }
// }

// // OTA Process
// bool downloadAndUpdate(String url)
// {
//     HTTPClient http;
//     http.begin(url);
//     http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

//     int httpCode = http.GET();

//     if (httpCode == 200)
//     {
//         int len = http.getSize();
//         WiFiClient *stream = http.getStreamPtr();

//         if (!Update.begin(len))
//         {
//             Serial.println("Update Begin Failed");
//             return false;
//         }

//         size_t written = Update.writeStream(*stream);
//         if (written == len)
//         {
//             Serial.println("Written : " + String(written) + " successfully");
//         }
//         else
//         {
//             Serial.println("Written only : " + String(written) + "/" + String(len) + ". Retry?");
//             return false;
//         }

//         if (Update.end())
//         {
//             if (Update.isFinished())
//             {
//                 Serial.println("Update successfully completed. Rebooting.");
//                 return true;
//             }
//             else
//             {
//                 Serial.println("Update not finished? Something went wrong!");
//                 return false;
//             }
//         }
//         else
//         {
//             Serial.println("Update.end() failed: " + String(Update.getError()));
//             return false;
//         }
//     }
//     else
//     {
//         Serial.printf("HTTP GET failed. Code: %d\n", httpCode);
//         return false;
//     }
// }

// // MQTT Callback
// void mqttCallback(char *topic, byte *payload, unsigned int length)
// {
//     String msg;
//     for (int i = 0; i < length; i++)
//     {
//         msg += (char)payload[i];
//     }
//     Serial.println("Payload: " + msg);

//     int urlIndex = msg.indexOf("fw_url");
//     if (urlIndex != -1)
//     {
//         int start = msg.indexOf("http", urlIndex);
//         int end = msg.indexOf("\"", start);
//         firmwareURL = msg.substring(start, end);
//         Serial.println("Received OTA URL: " + firmwareURL);
//         otaTriggered = true;
//     }
// }

// void setup()
// {
//     Serial.begin(115200);
//     connectToWiFi();
//     mqttClient.setCallback(mqttCallback);
//     connectToMQTT();
// }

// void loop()
// {
//     if (!mqttClient.connected())
//     {
//         connectToMQTT();
//     }
//     mqttClient.loop();

//     if (otaTriggered)
//     {
//         otaTriggered = false;
//         if (downloadAndUpdate(firmwareURL))
//         {
//             delay(2000);
//             ESP.restart();
//         }
//         else
//         {
//             Serial.println("OTA Failed.");
//         }
//     }
// }

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
const char *ssid = "Wokwi-GUEST";
const char *password = "";
const char *mqttServer = "demo.thingsboard.io";
const int mqttPort = 1883;
const char *mqttUsername = "DVLRXtb6oRXN8rq4INxO";
const char *mqttPassword = "";
const char *otaTopic = "v1/devices/me/attributes";
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

bool isFirmwareUpgradeTriggered = false;
void connectToWiFi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");
}
void connectToMQTT()
{
    mqttClient.setServer(mqttServer, mqttPort);
    while (!mqttClient.connected())
    {
        Serial.println("Connecting to MQTT...");
        if (mqttClient.connect("ESP32Client", mqttUsername, mqttPassword))
        {
            Serial.println("Connected to MQTT!");
            mqttClient.subscribe(otaTopic);
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void handleOTAUpdate(const uint8_t *firmwareData, size_t firmwareSize)
{
    ArduinoOTA.begin();
    ArduinoOTA.setHostname("ESP32Device");
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
Serial.printf("OTA Error[%u]: ", error);
if (error == OTA_AUTH_ERROR) {
Serial.println("Auth Failed");
} else if (error == OTA_BEGIN_ERROR) {
Serial.println("Begin Failed");
} else if (error == OTA_CONNECT_ERROR) {
Serial.println("Connect Failed");
} else if (error == OTA_RECEIVE_ERROR) {
Serial.println("Receive Failed");
} else if (error == OTA_END_ERROR) {
Serial.println("End Failed");
} });
    ArduinoOTA.begin();
}
void callback(char *topic, byte *payload, unsigned int length)
{
    if (strcmp(topic, otaTopic) == 0)
    {
        if (!isFirmwareUpgradeTriggered)
        {
            isFirmwareUpgradeTriggered = true;
            handleOTAUpdate(payload, length);
            Serial.println("Firmware upgrade triggered!");
        }
    }
}
void setup()
{
    Serial.begin(115200);
    connectToWiFi();
    connectToMQTT();
    mqttClient.setCallback(callback);

    ArduinoOTA.setPort(3232);
    ArduinoOTA.onStart([]()
                       { Serial.println("OTA Update started..."); });
    ArduinoOTA.onEnd([]()
                     {
Serial.println("OTA Update completed!");

isFirmwareUpgradeTriggered = false; });
    ArduinoOTA.begin();
}

void loop()
{
    if (!mqttClient.connected())
    {
        connectToMQTT();
    }
    mqttClient.loop();
    ArduinoOTA.handle();
}