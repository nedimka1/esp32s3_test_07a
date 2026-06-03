#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Preferences.h>

// ---------------- NVS ----------------
Preferences preferences;

// ---------------- config ----------------
String wifi_ssid;
String wifi_pass;

String mqtt_server;
String mqtt_user;
String mqtt_pass;

String device_name;

// ---------------- MQTT ----------------
WiFiClientSecure espClient;
PubSubClient client(espClient);

// ---------------- runtime ----------------
unsigned long lastSend = 0;
int counter = 0;

String topic_deneme01;

// ---------------- load config ----------------
bool loadConfig() {
  preferences.begin("config", true);

  wifi_ssid = preferences.getString("ssid", "");
  wifi_pass = preferences.getString("pass", "");

  mqtt_server = preferences.getString("mqtt_server", "");
  mqtt_user   = preferences.getString("mqtt_user", "");
  mqtt_pass   = preferences.getString("mqtt_pass", "");

  device_name = preferences.getString("device_name", "esp32");

  preferences.end();

  Serial.println("---- NVS CONFIG ----");
  Serial.println("SSID: " + wifi_ssid);
  Serial.println("MQTT: " + mqtt_server);

  // basit validation
  if (wifi_ssid == "" || mqtt_server == "") {
    Serial.println("CONFIG EKSİK!");
    return false;
  }

  return true;
}

// ---------------- WiFi ----------------
void wifiConnect() {
  WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi OK");
  Serial.println(WiFi.localIP());
}

// ---------------- MQTT ----------------
void mqttReconnect() {
  while (!client.connected()) {

    Serial.print("MQTT connecting...");

    String clientId = "ESP32-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (client.connect(clientId.c_str(),
                       mqtt_user.c_str(),
                       mqtt_pass.c_str())) {
      Serial.println("OK");
    } else {
      Serial.print("FAIL rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// ---------------- setup ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!loadConfig()) {
    Serial.println("CONFIG YOK -> cihaz durdu");
    while (1) delay(1000);
  }

  topic_deneme01 = "cam/" + device_name + "/deneme01";

  wifiConnect();

  espClient.setInsecure(); // HiveMQ Cloud test mode

  client.setServer(mqtt_server.c_str(), 8883);
}

// ---------------- loop ----------------
void loop() {

  if (!client.connected()) {
    mqttReconnect();
  }

  client.loop();

  if (millis() - lastSend > 30000) {
    lastSend = millis();
    counter++;

    String msg = device_name + " yasiyor 6 : " + String(counter);

    Serial.println("Sending: " + msg);

    client.publish(topic_deneme01.c_str(),
                   (uint8_t*)msg.c_str(),
                   msg.length(),
                   true);
  }
}