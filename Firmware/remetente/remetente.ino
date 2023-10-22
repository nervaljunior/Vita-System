#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>  // Linha relacionada ao MQTT
#include <ArduinoJson.h>
#include "secrets.h"
#include <Arduino_LSM6DS3.h> // Biblioteca do acelerômetro
//#include <HTTPClient.h>

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
//const char* serverAddress = "http://SEU_ENDERECO_API/dados-sensores/";  // Substitua pelo endereço do seu servidor FastAPI

// const char broker[] = "broker.mqttdashboard.com";  // Linha relacionada ao MQTT
// int port = 1883;  // Linha relacionada ao MQTT
// const char topic[] = "wokwi-weather";  // Linha relacionada ao MQTT
// const char* mqttUser = "";  // Linha relacionada ao MQTT
// const char* mqttPassword = "";  // Linha relacionada ao MQTT
// const char* clientId = "micropython-weather-demo";  // Linha relacionada ao MQTT

WiFiSSLClient wifiClient; // Usar WiFiSSLClient para conexões seguras com MQTT
// MqttClient mqttClient(wifiClient);  // Linha relacionada ao MQTT

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

  // Serial.print("Attempting to connect to the MQTT broker: ");
  // Serial.println(broker);  // Linhas relacionadas ao MQTT

  // if (!mqttClient.connect(broker, port)) {
  //   Serial.print("MQTT connection failed! Error code = ");
  //   Serial.println(mqttClient.connectError());
  //   while (1);
  // }

  // Serial.println("Connected to the MQTT broker!");  // Linhas relacionadas ao MQTT
  Serial.println();
}

void loop() {
  // if (!mqttClient.connected()) {  // Linha relacionada ao MQTT
  //   reconnect();  // Linha relacionada ao MQTT
  // }
  // mqttClient.poll();  // Linha relacionada ao MQTT

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
    int volume_corrente = analogRead(A0);
    int razao_ie = analogRead(A1);
    int frequencia = analogRead(A2);
    int fluxo_medio = analogRead(A3);

    // Crie um objeto JSON para armazenar os dados
    StaticJsonDocument<200> jsonDoc;

    // Preencha o objeto JSON com os dados
    jsonDoc["Volume_corrente"] = volume_corrente;
    jsonDoc["Razao_IE"] = razao_ie;
    jsonDoc["Frequencia"] = frequencia;
    jsonDoc["Fluxo_medio"] = fluxo_medio;
    jsonDoc["AcelerometroX"] = x;
    jsonDoc["AcelerometroY"] = y;
    jsonDoc["AcelerometroZ"] = z;

    // Crie um buffer para armazenar o JSON como uma string
    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);

    // Imprima o JSON no monitor Serial
    Serial.println("JSON enviado via MQTT:");
    Serial.println(jsonBuffer);

    /*HTTPClient http;
    http.begin(serverAddress);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonBuffer);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Resposta do servidor: " + response);
    } else {
      Serial.print("Erro na solicitação HTTP: ");
      Serial.println(httpResponseCode);
    }

    http.end();*/

    delay(5000);  // Envie os dados a cada 5 segundos
  }
}
/*
void reconnect() {  // Linha relacionada ao MQTT
   while (!mqttClient.connected()) {  // Linha relacionada ao MQTT
     Serial.print("Connecting to the MQTT server...");  // Linha relacionada ao MQTT
     if (mqttClient.connect("ArduinoClient")) {  // Linha relacionada ao MQTT
       Serial.println("Connected");  // Linha relacionada ao MQTT
     } else {
       Serial.println("Connection failed. Trying again in 5 seconds...");  // Linha relacionada ao MQTT
       delay(5000);  // Linha relacionada ao MQTT
     }
   }
 }
*/