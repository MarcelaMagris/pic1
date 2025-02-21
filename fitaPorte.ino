#include <Adafruit_NeoPixel.h>

// Definição dos pinos e quantidade de LEDs
#define PINO_FITA 8   // Pino onde a WS2812B está conectada
#define NUM_LEDS 3    // Número de LEDs na fita

// Inicializa a fita LED
Adafruit_NeoPixel fita(NUM_LEDS, PINO_FITA, NEO_GRB + NEO_KHZ800);

// Variável para determinar o porte da árvore
int porteArvore = 3;  // Altere esse valor para 1, 2 ou 3

void setup() {
  fita.begin();
  fita.show(); // Inicializa os LEDs apagados
}

void loop() {
  // Apaga todos os LEDs antes de atualizar
  fita.clear();

  // Define a cor dos LEDs conforme o porte da árvore
  if (porteArvore == 1) {
    fita.setPixelColor(0, fita.Color(255, 182, 193)); // Rosinha claro
  } else if (porteArvore == 2) {
    fita.setPixelColor(1, fita.Color(255, 105, 180)); // Rosa
  } else if (porteArvore == 3) {
    fita.setPixelColor(2, fita.Color(255, 20, 147));  // Rosa mais forte possível
  }

  fita.show(); // Atualiza os LEDs
  delay(1000); // Pequeno atraso para evitar flickering
}
