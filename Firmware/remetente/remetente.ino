#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>  
#include <ArduinoJson.h>
#include "secrets.h"
#include <Arduino_LSM6DS3.h> 


char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;


 const char broker[] = "broker.mqttdashboard.com";  
 int port = 1883;  
 const char topic[] = "wokwi-weather";  
 const char* mqttUser = "";  
 const char* mqttPassword = "";  
 const char* clientId = "micropython-weather-demo";  

WiFiSSLClient wifiClient; // Usar WiFiSSLClient para conexões seguras com MQTT
 MqttClient mqttClient(wifiClient);  

const long interval = 8000;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }


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


    float x, y, z;
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
    }


    int volume_corrente = analogRead(A0);
    int razao_ie = analogRead(A1);
    int frequencia = analogRead(A2);
    int fluxo_medio = analogRead(A3);


    //StaticJsonDocument<200> jsonDoc;
    DynamicJsonDocument jsonDoc(200);


    jsonDoc["Volume_corrente"] = volume_corrente;
    jsonDoc["Razao_IE"] = razao_ie;
    jsonDoc["Frequencia"] = frequencia;
    jsonDoc["Fluxo_medio"] = fluxo_medio;
    jsonDoc["AcelerometroX"] = x;
    jsonDoc["AcelerometroY"] = y;
    jsonDoc["AcelerometroZ"] = z;


    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);


    Serial.println("JSON enviado via MQTT:");
    Serial.println(jsonBuffer);


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
