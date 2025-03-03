#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>    // Biblioteca do sensor AHT21
#include "ScioSense_ENS160.h"  // Biblioteca do sensor ENS160
#include <Adafruit_NeoPixel.h>

#define PIN_LED_TEMP 25
#define PIN_LED_AQI 33
#define NUM_LEDS 9

Adafruit_NeoPixel stripTemp(NUM_LEDS, PIN_LED_TEMP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripAQI(NUM_LEDS, PIN_LED_AQI, NEO_GRB + NEO_KHZ800);

// Configuração do WiFi
const char* ssid = "RYANNE_AP106_2Ghz";
const char* password = "Norway106*";

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

// Função para definir a cor dos LEDs da fita de temperatura
void setCorTemp(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    stripTemp.setPixelColor(i, stripTemp.Color(r, g, b));
  }
  stripTemp.show();
}

// Função para definir a cor dos LEDs da fita de qualidade do ar
void setCorAQI(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    stripAQI.setPixelColor(i, stripAQI.Color(r, g, b));
  }
  stripAQI.show();
}

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

  // Inicializa as fitas LED
  stripTemp.begin();
  stripTemp.show();  // Inicializa os LEDs apagados
  stripAQI.begin();
  stripAQI.show();  // Inicializa os LEDs apagados

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
  String payload = String(temperatura) + "," + String(umidade) + "," + String(eCO2) + ";";// + String(tvoc) + "/" + String(aqi) + ";";
  Serial.println("Enviando para MQTT: " + payload);

  if (temperatura >= 35) {
    setCorTemp(255, 0, 0);  // Vermelho (muito quente)
  } else if (temperatura >= 30) {
    setCorTemp(255, 50, 0);  // Laranja (quente)
  } else if (temperatura >= 25) {
    setCorTemp(255, 120, 0);  // Amarelo (morno)
  } else if (temperatura >= 20) {
    setCorTemp(0, 255, 255);  // Azul claro (frio)
  } else {
    setCorTemp(0, 0, 255);  // Azul (muito frio)
  }

  // Define a cor da fita LED de qualidade do ar (AQI)
  if (aqi == 5) {
    setCorAQI(255, 0, 255);  // Roxo (muito ruim)
  } else if (aqi == 4) {
    setCorAQI(255, 0, 0);  // Vermelho (ruim)
  } else if (aqi == 3) {
    setCorAQI(255, 50, 0);  // Laranja (moderado)
  } else if (aqi == 2) {
    setCorAQI(0, 255, 0);  // Verde (bom)
  } else if (aqi == 1) {
    setCorAQI(0, 0, 255);  // Azul (muito bom)
  }


  // Publica os dados no MQTT
  client.publish(mqttTopic, payload.c_str());

  // Aguarda 5 segundos antes da próxima leitura
  delay(5000);
}
