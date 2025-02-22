#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>    // Biblioteca do sensor AHT21
#include "ScioSense_ENS160.h"  // Biblioteca do sensor ENS160

// Configuração do WiFi
const char* ssid = "WIFI";
const char* password = "SENHA";

// Configuração do MQTT
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttTopic = "pic1UFES/arbotec";

// Instâncias dos sensores
Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

// Configuração do cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Variáveis para armazenar leituras
float temperatura;
int umidade;
int eCO2, aqi, tvoc;

void setup() {
  Serial.begin(115200);

  // Conectar ao WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  // Conectar ao MQTT
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32_Client")) {
      Serial.println("Conectado ao MQTT!");
    } else {
      Serial.print("Falha, código: ");
      Serial.println(client.state());
      delay(5000);
    }
  }

  // Inicializa o sensor AHT21
  if (!aht.begin()) {
    Serial.println("Erro ao detectar o AHT21! Verifique a conexão.");
    while (1);
  }

  // Inicializa o sensor ENS160
  if (!ens160.begin()) {
    Serial.println("Erro ao detectar o ENS160! Verifique a conexão.");
    while (1);
  }
  ens160.setMode(ENS160_OPMODE_STD); // Modo padrão de medição
}

void loop() {
  // Reconectar se necessário
  if (!client.connected()) {
    while (!client.connect("ESP32_Client")) {
      Serial.print("Reconectando ao MQTT...");
      delay(5000);
    }
  }

  // Leitura do sensor AHT21
  sensors_event_t humidityEvent, tempEvent;
  aht.getEvent(&humidityEvent, &tempEvent);
  temperatura = tempEvent.temperature;
  umidade = humidityEvent.relative_humidity;

  // Envia os dados ao ENS160 para melhorar a precisão
  ens160.set_envdata(temperatura, umidade);
  ens160.measure(true);

  // Captura os valores do ENS160
  aqi = ens160.getAQI();
  tvoc = ens160.getTVOC();
  eCO2 = ens160.geteCO2();

  // Exibe as leituras no monitor serial
  Serial.print("Temperatura: "); Serial.print(temperatura); Serial.println(" °C");
  Serial.print("Umidade: "); Serial.print(umidade); Serial.println("% rH");
  Serial.print("AQI: "); Serial.print(aqi); Serial.print("\t");
  Serial.print("TVOC: "); Serial.print(tvoc); Serial.print(" ppb\t");
  Serial.print("eCO2: "); Serial.print(eCO2); Serial.println(" ppm");

  // Monta a string no formato especificado
  String payload = String(temperatura) + "/" + String(umidade) + "/" + String(eCO2) + "/" + String(tvoc) + "/" + String(aqi) + ";";
  Serial.println("Enviando para MQTT: " + payload);

  // Publica os dados no MQTT
  client.publish(mqttTopic, payload.c_str());

  // Aguarda 5 segundos antes da próxima leitura
  delay(5000);
}
