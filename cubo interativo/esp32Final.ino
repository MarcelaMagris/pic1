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

// Configuração de múltiplas redes WiFi
const char* ssid1 = "RYANNE_AP106_2Ghz";
const char* password1 = "Norway106*";
const char* ssid2 = "PIC2-2G";
const char* password2 = "engcomp@ufes";

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
double temperatura;
int umidade;
int eCO2, aqi, tvoc;

// Função para conectar ao WiFi alternando entre as redes
void connectWiFi() {
    int attempts;
    bool connected = false;
    while (!connected) {
        // Tenta conectar no primeiro SSID
        WiFi.begin(ssid1, password1);
        Serial.print("Conectando ao WiFi1...");
        for (attempts = 0; attempts < 5; attempts++) {
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("\nConectado ao WiFi1!");
                connected = true;
                break;
            }
            delay(1000);
            Serial.print(".");
        }
        
        // Se não conectou, tenta o segundo SSID
        if (!connected) {
            WiFi.begin(ssid2, password2);
            Serial.print("\nConectando ao WiFi2...");
            for (attempts = 0; attempts < 5; attempts++) {
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("\nConectado ao WiFi2!");
                    connected = true;
                    break;
                }
                delay(1000);
                Serial.print(".");
            }
        }
    }
}

// Função para conectar ao MQTT
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
        Serial.println("Erro ao detectar o AHT21! Verifique a conexão.");
        while (1);
    }
    if (!ens160.begin()) {
        Serial.println("Erro ao detectar o ENS160! Verifique a conexão.");
        while (1);
    }
    ens160.setMode(ENS160_OPMODE_STD);
}

// Função para definir a cor dos LEDs
void setCor(Adafruit_NeoPixel &strip, int r, int g, int b) {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

void setup() {
    Serial.begin(115200);
    
    connectWiFi();
    connectMQTT();
    
    // Inicializa as fitas LED
    stripTemp.begin();
    stripTemp.show();  // Inicializa os LEDs apagados
    stripAQI.begin();
    stripAQI.show();  // Inicializa os LEDs apagados
    
    initSensors();
}

void loop() {
    // Verifica conexão MQTT e reconecta se necessário
    if (!client.connected()) {
        connectMQTT();
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
    Serial.print("AQI: "); Serial.print(aqi);
    Serial.print(" TVOC: "); Serial.print(tvoc); Serial.print(" ppb ");
    Serial.print("eCO2: "); Serial.print(eCO2); Serial.println(" ppm");
    
    // Monta a string para o MQTT
    String payload = String(temperatura) + "," + String(umidade) + "," + String(eCO2) + ";";
    Serial.println("Enviando para MQTT: " + payload);
    client.publish(mqttTopic, payload.c_str());

    // Define a cor da fita LED de temperatura
    if (temperatura >= 35) setCor(stripTemp, 255, 0, 0);
    else if (temperatura >= 30) setCor(stripTemp, 255, 50, 0);
    else if (temperatura >= 25) setCor(stripTemp, 255, 120, 0);
    else if (temperatura >= 20) setCor(stripTemp, 0, 255, 255);
    else setCor(stripTemp, 0, 0, 255);

    // Define a cor da fita LED de qualidade do ar (AQI)
    if (aqi == 5) setCor(stripAQI, 255, 0, 255);
    else if (aqi == 4) setCor(stripAQI, 255, 0, 0);
    else if (aqi == 3) setCor(stripAQI, 255, 50, 0);
    else if (aqi == 2) setCor(stripAQI, 0, 255, 0);
    else if (aqi == 1) setCor(stripAQI, 0, 0, 255);
    
    delay(5000); // Aguarda 5 segundos para a próxima leitura
}