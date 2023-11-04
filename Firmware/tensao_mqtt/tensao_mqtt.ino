#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include <Arduino_LSM6DS3.h> 

int corrente_inst[300];
int pino_sensor = A2;

float corrente_pico;
float corrente_eficaz;
float tensao_rms;
float tensao_pico;
float frequencia_sinal;

double maior_valor = 0;
double corrente_valor = 0;

// Wi-Fi
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// Broker
const char* MQTT_SERVER = "test.mosquitto.org";
const char* mqtt_user = "";
const char* mqtt_password = "";

// Porta
const int MQTT_PORT = 1883;

// Cliente
const char* MQTT_CLIENT = "IeCACMonitor";

const char* MQTT_AC_DATA_TOPIC = "IeCACMonitor/redeAC/dados";
const char* mqtt_topic = "wokwi-weather";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const int potPin = 37;
const int zmptPin = 13;
const int ssrPin = 2;
float Vref = 3.3;

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("Mensagem: " + message);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se reconectar ao broker MQTT...");
    
    if (client.connect("arduinoClient", mqtt_user, mqtt_password)) {
      Serial.println("Conectado com sucesso");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println("Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(pino_sensor, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int acRead = analogRead(A0);

  maior_valor = 0;
  for (int i = 0; i < 300; i++) {
    corrente_inst[i] = analogRead(pino_sensor);
  }
  for (int i = 0; i < 300; i++) {
    if (maior_valor < corrente_inst[i]) {
      maior_valor = corrente_inst[i];
    }
  }

  tensao_pico = map(maior_valor, 500, 661, 0, 313);
  tensao_rms = tensao_pico / 1.4;

  String jsonPayload = "{";
  jsonPayload += "\"tensao_rms\":" + String(tensao_rms, 2) + ",";
  jsonPayload += "\"tensao_pico\":" + String(tensao_pico, 2) + ",";
  jsonPayload += "\"acRead\:" + String(acRead) + "}";

  client.publish(MQTT_AC_DATA_TOPIC, jsonPayload.c_str());

  delay(1000);
}





/*#include <Wire.h>
//#include <WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>  
#include <ArduinoJson.h>
#include "secrets.h"
#include <Arduino_LSM6DS3.h> 


int corrente_inst[300];
int pino_sensor = A2;

float corrente_pico;
float corrente_eficaz;
float tensao_rms;
float tensao_pico;
float frequencia_sinal;

double maior_valor = 0;
double corrente_valor = 0;

// Wi-Fi
char ssid[] = SECRET_SSID;
char password[] = SECRET_PASS;

// Broker
const char* MQTT_SERVER = "test.mosquitto.org";
const char* mqtt_user = ""; 
const char* mqtt_password = ""; 


// Porta
const int MQTT_PORT = 1883;

// Cliente
const char* MQTT_CLIENT = "IeCACMonitor";

const char* MQTT_AC_DATA_TOPIC = "IeCACMonitor/redeAC/dados";
const char* mqtt_topic = "wokwi-weather";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const int potPin = 37;
const int zmptPin = 13;
const int ssrPin = 2;
float Vref = 3.3;

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("Mensagem: " + message);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se reconectar ao broker MQTT...");
//MQTT_CLIENT
    if (client.connect("arduinoClient",mqtt_user, mqtt_password)) {
      Serial.println("Conectado com sucesso");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println("Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(pino_sensor, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  client.setServer(MQTT_SERVER, MQTT_PORT);

 }

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int acRead = analogRead(A0);

  maior_valor = 0;
  for (int i = 0; i < 300; i++) {
    corrente_inst[i] = analogRead(pino_sensor);
  }
  for (int i = 0; i < 300; i++) {
    if (maior_valor < corrente_inst[i]) {
      maior_valor = corrente_inst[i];
    }
  }

  tensao_pico = map(maior_valor, 500, 661, 0, 313);
  tensao_rms = tensao_pico / 1.4;

  String jsonPayload = "{";
  jsonPayload += "\"tensao_rms\":" + String(tensao_rms, 2) + ",";
  jsonPayload += "\"tensao_pico\":" + String(tensao_pico, 2) + ",";
  jsonPayload += "\"acRead\":" + String(acRead) + "}";

  client.publish(MQTT_AC_DATA_TOPIC, jsonPayload.c_str());

  delay(1000);
}*/
