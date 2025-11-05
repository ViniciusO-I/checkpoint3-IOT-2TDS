#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- Identificadores (apenas exemplo para o payload)
const char* ID       = "RM559611";
const char* moduleID = "VINICIUS-OLIVEIRA";

// --- Wi-Fi (Wokwi usa SSID Wokwi-GUEST, sem senha)
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// --- MQTT (sua VM no Azure)
const char* BROKER_MQTT  = "172.200.90.234";
const uint16_t BROKER_PORT = 1883;
const char* mqttUser     = "admin";
const char* mqttPassword = "Fiap@2tdsvms";
const char* TOPIC        = "teste/topico";

WiFiClient espClient;
PubSubClient mqtt(espClient);
unsigned long lastSend = 0;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi OK. IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  if (mqtt.connected()) return;
  mqtt.setServer(BROKER_MQTT, BROKER_PORT);
  Serial.print("Conectando ao MQTT");
  while (!mqtt.connected()) {
    if (mqtt.connect("esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX), mqttUser, mqttPassword)) {
      Serial.println("\nMQTT conectado!");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}

void publishJson() {
  StaticJsonDocument<200> doc;
  doc["id"]       = ID;
  doc["moduleID"] = moduleID;
  doc["ts"]       = (uint32_t) (millis()/1000);
  doc["value"]    = random(20, 41); // um valor qualquer (20..40)

  char payload[256];
  size_t n = serializeJson(doc, payload);
  bool ok = mqtt.publish(TOPIC, payload, n);
  Serial.print("Publish -> ");
  Serial.println(ok ? payload : "FAILED");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  connectWiFi();
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  if (millis() - lastSend > 5000) { // a cada 5s
    publishJson();
    lastSend = millis();
  }
}
