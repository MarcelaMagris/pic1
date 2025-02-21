#include <Wire.h>
#include <Adafruit_AHTX0.h>       // Biblioteca do sensor AHT20
#include "ScioSense_ENS160.h"     // Biblioteca do sensor ENS160
#include <Adafruit_NeoPixel.h>    // Biblioteca para controle da fita WS2812B

// Definições dos pinos para as fitas LED
#define PIN_LED_TEMP 6  // Pino da fita WS2812B que indica temperatura
#define PIN_LED_AQI 7   // Pino da fita WS2812B que indica qualidade do ar
#define NUM_LEDS 9      // Número de LEDs em cada fita

// Definição dos pinos e quantidade de LEDs
#define PINO_FITA_PORTE 8   // Pino onde a WS2812B está conectada
#define NUM_LEDS_PORTE 3    // Número de LEDs na fita
#define BOTAO 2       // Pino do botão

// Instâncias das fitas de LED
Adafruit_NeoPixel stripTemp(NUM_LEDS, PIN_LED_TEMP, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripAQI(NUM_LEDS, PIN_LED_AQI, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fita(NUM_LEDS_PORTE, PINO_FITA_PORTE, NEO_GRB + NEO_KHZ800);

// Sensores
Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1); // Endereço I2C do ENS160

// Variáveis para armazenar leituras
float temperatura;
int umidade;
int eCO2, aqi, tvoc;

// Variáveis para controle
int porteArvore = 2;  // Altere esse valor para 1, 2 ou 3 manualmente
bool estadoBotaoAnterior = LOW;
bool atualizarLeds = false; // Indica se os LEDs devem ser atualizados

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
  Serial.begin(9600);
  pinMode(BOTAO, INPUT); // Configura o pino do botão como entrada
  
  // Inicializa as fitas LED
  stripTemp.begin();
  stripTemp.show(); // Inicializa os LEDs apagados
  stripAQI.begin();
  stripAQI.show(); // Inicializa os LEDs apagados
  fita.begin();
  fita.show(); // Inicializa os LEDs apagados

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

  // Define a cor da fita LED de temperatura
  if (temperatura >= 35) {
    setCorTemp(255, 0, 0);      // Vermelho (muito quente)
  } else if (temperatura >= 30) {
    setCorTemp(255, 50, 0);     // Laranja (quente)
  } else if (temperatura >= 25) {
    setCorTemp(255, 120, 0);    // Amarelo (morno)
  } else if (temperatura >= 20) {
    setCorTemp(0, 255, 255);    // Azul claro (frio)
  } else {
    setCorTemp(0, 0, 255);      // Azul (muito frio)
  }

  // Define a cor da fita LED de qualidade do ar (AQI)
  if (aqi == 5) {
    setCorAQI(255, 0, 255);   // Roxo (muito ruim)
  } else if (aqi == 4) {
    setCorAQI(255, 0, 0);     // Vermelho (ruim)
  } else if (aqi == 3) {
    setCorAQI(255, 50, 0);    // Laranja (moderado)
  } else if (aqi == 2) {
    setCorAQI(0, 255, 0);     // Verde (bom)
  } else if (aqi == 1) {
    setCorAQI(0, 0, 255);     // Azul (muito bom)
  }

  bool estadoBotaoAtual = digitalRead(BOTAO);

  // Detecta se o botão foi pressionado (transição de LOW para HIGH)
  if (estadoBotaoAtual == HIGH && estadoBotaoAnterior == LOW) {
    atualizarLeds = !atualizarLeds; // Alterna o estado da exibição
  }
  estadoBotaoAnterior = estadoBotaoAtual; // Atualiza estado do botão

  // Se deve atualizar, muda os LEDs
  if (atualizarLeds) {
    fita.clear();

    if (porteArvore == 1) {
      fita.setPixelColor(0, fita.Color(255, 182, 193)); // Rosinha claro
    } else if (porteArvore == 2) {
      fita.setPixelColor(1, fita.Color(255, 105, 180)); // Rosa
    } else if (porteArvore == 3) {
      fita.setPixelColor(2, fita.Color(255, 20, 147));  // Rosa mais forte
    }

    fita.show(); // Atualiza os LEDs
  } else {
    fita.clear();
    fita.show(); // Mantém os LEDs apagados até pressionar novamente
  }
  
  delay(5000); // Aguarda 5 segundos antes da próxima leitura
}
