#include <Adafruit_NeoPixel.h>

#define PIN_LED 6       // Pino conectado à fita WS2812B
#define NUM_LEDS 10     // Número de LEDs na fita

Adafruit_NeoPixel strip(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// Variável da temperatura (simulada)
float temperatura = 18; // Altere para testar diferentes temperaturas

// Função para definir a cor da fita
void setCor(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void setup() {
  strip.begin();
  strip.show(); // Inicializa a fita apagada
}

void loop() {
  if (temperatura >= 35) {
    setCor(255, 0, 0);    // Vermelho
  } else if (temperatura >= 30) {
    setCor(255, 50, 0);  // Laranja
  } else if (temperatura >= 25) {
    setCor(255, 120, 0);    // Amarelo
  } else if (temperatura >= 20) {
    setCor(0, 255, 255);  // Azul claro
  } else {
    setCor(0, 0, 255);    // Azul
  }

  delay(1000); // Atualiza a cor a cada segundo
}