#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include <Arduino_LSM6DS3.h> 

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char broker[] = "test.mosquitto.org";
int port = 1883;
const char topic[] = "giros";
const char* mqttUser = "";
const char* mqttPassword = "";
const char* clientId = "";

WiFiSSLClient wifiClient; 
MqttClient mqttClient(wifiClient);

const long interval = 8000;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  // Inicializa o acelerômetro
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }

  Serial.println("Connected to Wi-Fi");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }

  Serial.println("Connected to the MQTT broker!");
  Serial.println();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.poll();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Leitura dos valores do acelerômetro
    float x, y, z;
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
    }

    //aqui eu tenho que pegar os dados que ja tem na programação do arduino
    // Leitura dos valores dos sensores analógicos
    /*int volume_corrente = analogRead(A0);
    int razao_ie = analogRead(A1);
    int frequencia = analogRead(A2);
    int fluxo_medio = analogRead(A3);*/

    // Crie um objeto JSON para armazenar os dados
    //StaticJsonDocument<200> jsonDoc;
    DynamicJsonDocument jsonDoc(200);

    // Preencha o objeto JSON com os dados
    /*jsonDoc["Volume_corrente"] = volume_corrente;
    jsonDoc["Razao_IE"] = razao_ie;
    jsonDoc["Frequencia"] = frequencia;
    jsonDoc["Fluxo_medio"] = fluxo_medio;*/
    jsonDoc["AcelerometroX"] = x;
    jsonDoc["AcelerometroY"] = y;
    jsonDoc["AcelerometroZ"] = z;

    // Crie um buffer para armazenar o JSON como uma string
    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);

    // Imprima o JSON no monitor Serial
    Serial.println("JSON enviado via MQTT:");
    Serial.println(jsonBuffer);

    // Publique o JSON no tópico MQTT
    mqttClient.beginMessage(topic);
    mqttClient.print(jsonBuffer);
    mqttClient.endMessage();

    Serial.println("JSON enviado via MQTT:");
    Serial.println(jsonBuffer);

    delay(5000);
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to the MQTT server...");
    if (mqttClient.connect("ArduinoClient")) {
      Serial.println("Connected");
    } else {
      Serial.println("Connection failed. Trying again in 5 seconds...");
      delay(5000);
    }
  }
}
