#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"
#include <cmath>
#include "heltec.h"
#include <ArduinoJson.h>

#define BAND 115200

WiFiClient espClient;
PubSubClient client(espClient);

const int windowSize = 6;
int sensorReadings[windowSize];
int currentReadings[windowSize];
int currentIndex = 0;
int id = 24;

unsigned long printPeriod = 1000;
unsigned long previousMillis = 0;

StaticJsonDocument<200> jsonDoc;  
StaticJsonDocument<200> giroscopio;

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT server...");
    if (client.connect("esp32-client", mqtt_user, mqtt_password)) {
      Serial.println("Connected!");
      client.subscribe("button");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retry in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(BAND);
  connectToWiFi();
  connectToMQTT();
}

void loop() {
  client.loop();
  jsonDoc["id"] = id;
  jsonDoc["luminosidade"] = random(0, 100);
  jsonDoc["humidade"] = random(0, 100);
  jsonDoc["temperatura"] = random(-10, 40);
  jsonDoc["altitude"] = random(0, 100);
  giroscopio["x"] = random(0, 100);
  giroscopio["y"] = random(-10, 40);
  giroscopio["z"] = random(-10, 40);
  jsonDoc["giroscopio"] = giroscopio;
  
  char jsonBuffer[200];
  serializeJson(jsonDoc, jsonBuffer);

  Serial.println("JSON enviado via MQTT:");
  Serial.println(jsonBuffer);

  client.publish("messages", jsonBuffer);

  delay(10000);  
}
