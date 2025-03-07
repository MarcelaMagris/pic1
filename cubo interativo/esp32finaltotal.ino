// Inclui as bibliotecas necessárias
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>    // Biblioteca para o sensor AHT21 (Temperatura e Umidade)
#include "ScioSense_ENS160.h"  // Biblioteca para o sensor ENS160 (Qualidade do Ar)
#include <Adafruit_NeoPixel.h> // Biblioteca para controle da fita de LEDs

// Definições de hardware
#define PIN_LED_TEMP 25 // Pino da fita LED para temperatura
#define PIN_LED_AQI 33  // Pino da fita LED para qualidade do ar
#define NUM_LEDS 9      // Número total de LEDs em cada fita

// Instâncias dos objetos para controle das fitas LED
Adafruit_NeoPixel stripTemp(NUM_LEDS, PIN_LED_TEMP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripAQI(NUM_LEDS, PIN_LED_AQI, NEO_GRB + NEO_KHZ800);

// Configuração das redes WiFi
const char* ssid1 = "PIC2-2.4G";
const char* password1 = "engcomp@ufes";
const char* ssid2 = "PIC2-2.4G";//PIC2-2.4G
const char* password2 = "engcomp@ufes";//engcomp@ufes

// Configuração do servidor MQTT
const char* mqttServer = "test.mosquitto.org";//broker.hivemq.com
const int mqttPort = 1883;
const char* mqttTopic = "pic1UFES/arbotec/InfoUfes";

// Instâncias dos sensores
Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

// Cliente WiFi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Variáveis globais para armazenar leituras
double temperatura;
int umidade;
int eCO2, aqi, tvoc;

// Função para conectar ao WiFi
void connectWiFi() {
    int attempts;
    bool connected = false;
    while (!connected) {
        WiFi.begin(ssid1, password1);
        Serial.print("Conectando ao WiFi1...");
        for (attempts = 0; attempts < 5; attempts++) {
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("\nConectado ao WiFi1!");
                connected = true;
                return;
            }
            delay(1000);
        }
        WiFi.begin(ssid2, password2);
        Serial.print("\nConectando ao WiFi2...");
        for (attempts = 0; attempts < 5; attempts++) {
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("\nConectado ao WiFi2!");
                return;
            }
            delay(1000);
        }
    }
}

// Função para conectar ao servidor MQTT
void connectMQTT() {
    client.setServer(mqttServer, mqttPort);
    while (!client.connected()) {
        Serial.print("Conectando ao MQTT...");
        if (client.connect("ESP32_Client")) {
            Serial.println(" Conectado ao MQTT!");
        } else {
            Serial.print(" Falha, código: ");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

// Função para inicializar os sensores
void initSensors() {
    if (!aht.begin()) {
        Serial.println("Erro ao detectar o AHT21!");
        while (1);
    }
    if (!ens160.begin()) {
        Serial.println("Erro ao detectar o ENS160!");
        while (1);
    }
    ens160.setMode(ENS160_OPMODE_STD);
}

// Função para configurar as cores da fita de LEDs
void setCor(Adafruit_NeoPixel &strip, int r, int g, int b) {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

// Função para coletar dados dos sensores
void readSensors() {
    sensors_event_t humidityEvent, tempEvent;
    aht.getEvent(&humidityEvent, &tempEvent);
    temperatura = tempEvent.temperature;
    umidade = humidityEvent.relative_humidity;
    ens160.set_envdata(temperatura, umidade);
    ens160.measure(true);
    aqi = ens160.getAQI();
    tvoc = ens160.getTVOC();
    eCO2 = ens160.geteCO2();
}

// Função para exibir leituras no monitor serial
void printSensorData() {
    Serial.println();
    Serial.print("Temperatura: "); Serial.print(temperatura); Serial.println(" °C");
    Serial.print("Umidade: "); Serial.print(umidade); Serial.println("% rH");
    Serial.print("AQI: "); Serial.print(aqi);
    Serial.print(" TVOC: "); Serial.print(tvoc); Serial.print(" ppb ");
    Serial.print("eCO2: "); Serial.print(eCO2); Serial.println(" ppm");
}

// Função para definir cores das fitas LED com base nos valores lidos
void updateLEDs() {
    if (temperatura >= 35) setCor(stripTemp, 255, 0, 0); // Vermelho (muito quente)
    else if (temperatura >= 30) setCor(stripTemp, 255, 50, 0); // Laranja forte
    else if (temperatura >= 25) setCor(stripTemp, 255, 120, 0); // Laranja fraco
    else if (temperatura >= 20) setCor(stripTemp, 0, 255, 255); // Azul claro (frio)
    else setCor(stripTemp, 0, 0, 255); // Azul escuro (muito frio)

    if (aqi == 5) setCor(stripAQI, 255, 0, 255); // Roxo (péssima qualidade do ar)
    else if (aqi == 4) setCor(stripAQI, 255, 0, 0); // Vermelho (ruim)
    else if (aqi == 3) setCor(stripAQI, 255, 50, 0); // Laranja (moderado)
    else if (aqi == 2) setCor(stripAQI, 0, 255, 0); // Verde (bom)
    else if (aqi == 1) setCor(stripAQI, 0, 0, 255); // Azul (excelente)
}

// Função para enviar dados via MQTT
void sendMQTT() {
    String payload = String(temperatura) + "," + String(umidade) + "," + String(eCO2) + ";";
    Serial.println("Enviando para MQTT: " + payload);
    client.publish(mqttTopic, payload.c_str());
}

void setup() {
    Serial.begin(115200);
    connectWiFi();
    connectMQTT();
    stripTemp.begin();
    stripAQI.begin();
    initSensors();
}

void loop() {
    if (!client.connected()) connectMQTT();
    readSensors();
    printSensorData();
    updateLEDs();
    sendMQTT();
    delay(20000);
}
