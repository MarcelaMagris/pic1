#include <Wire.h>
#include <Adafruit_AHTX0.h>       // Biblioteca do sensor AHT20
#include "ScioSense_ENS160.h"     // Biblioteca do sensor ENS160
#include <Adafruit_NeoPixel.h>    // Biblioteca para controle da fita WS2812B

// Definições da fita LED
#define PIN_LED 6       // Pino onde a WS2812B está conectada
#define NUM_LEDS 9      // Número de LEDs na fita

Adafruit_NeoPixel strip(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// Sensores
Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1); // Endereço I2C do ENS160

// Variáveis para armazenar leituras
float temperatura;
int umidade;
int eCO2, aqi, tvoc;

// Função para definir a cor dos LEDs com base na temperatura
void setCor(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Inicializa os LEDs apagados

  // Inicializa o sensor AHT20
  if (!aht.begin()) {
    Serial.println("Erro ao detectar o AHT20! Verifique a conexão.");
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
  // Leitura do sensor AHT20
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

  // Define a cor da fita LED de acordo com a temperatura
  if (temperatura >= 35) {
    setCor(255, 0, 0);      // Vermelho (muito quente)
  } else if (temperatura >= 30) {
    setCor(255, 50, 0);     // Laranja (quente)
  } else if (temperatura >= 25) {
    setCor(255, 120, 0);    // Amarelo (morno)
  } else if (temperatura >= 20) {
    setCor(0, 255, 255);    // Azul claro (frio)
  } else {
    setCor(0, 0, 255);      // Azul (muito frio)
  }

  delay(5000); // Aguarda 5 segundos antes da próxima leitura
}
