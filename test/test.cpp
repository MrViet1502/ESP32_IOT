
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>       // 👈 Thêm dòng này
#include <WiFiClientSecure.h> // 👈 Nếu dùng HTTPS
#include <ArduinoOTA.h>

// WiFi
const char *ssid = "viet";
const char *password = "20252025";

// MQTT CoreIoT (ThingsBoard)
const char *mqttServer = "app.coreiot.io";
const int mqttPort = 1883;
const char *mqttUsername = "gB69jhkhOWD1wEYj6mm7";
const char *mqttPassword = "";
const char *otaTopic = "v1/devices/me/attributes";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
String firmwareURL = "";
bool otaTriggered = false;

// WiFi Connect
void connectToWiFi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connecting to WiFi...");
        delay(1000);
    }
    Serial.println("Connected to WiFi");
}

// MQTT Connect
void connectToMQTT()
{
    mqttClient.setServer(mqttServer, mqttPort);
    while (!mqttClient.connected())
    {
        Serial.println("Connecting to MQTT...");
        if (mqttClient.connect("ESP32Client", mqttUsername, mqttPassword))
        {
            Serial.println("Connected to MQTT");
            mqttClient.subscribe(otaTopic);
        }
        else
        {
            Serial.print("MQTT failed, rc=");
            Serial.println(mqttClient.state());
            delay(5000);
        }
    }
}

// OTA Process
bool downloadAndUpdate(String url)
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

// MQTT Callback
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String msg;
    for (int i = 0; i < length; i++)
    {
        msg += (char)payload[i];
    }
    Serial.println("Payload: " + msg);

    int urlIndex = msg.indexOf("fw_url");
    if (urlIndex != -1)
    {
        int start = msg.indexOf("http", urlIndex);
        int end = msg.indexOf("\"", start);
        firmwareURL = msg.substring(start, end);
        Serial.println("Received OTA URL: " + firmwareURL);
        otaTriggered = true;
    }
}

void setup()
{
    Serial.begin(115200);
    connectToWiFi();
    mqttClient.setCallback(mqttCallback);
    connectToMQTT();
}

void loop()
{
    if (!mqttClient.connected())
    {
        connectToMQTT();
    }
    mqttClient.loop();

    if (otaTriggered)
    {
        otaTriggered = false;
        if (downloadAndUpdate(firmwareURL))
        {
            delay(2000);
            ESP.restart();
        }
        else
        {
            Serial.println("OTA Failed.");
        }
    }
}
