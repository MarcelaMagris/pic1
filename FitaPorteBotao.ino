#include <Adafruit_NeoPixel.h>

// Definição dos pinos e quantidade de LEDs
#define PINO_FITA 8   // Pino onde a WS2812B está conectada
#define NUM_LEDS 3    // Número de LEDs na fita
#define BOTAO 2       // Pino do botão

// Inicializa a fita LED
Adafruit_NeoPixel fita(NUM_LEDS, PINO_FITA, NEO_GRB + NEO_KHZ800);

// Variáveis para controle
int porteArvore = 1;  // Altere esse valor para 1, 2 ou 3 manualmente
bool estadoBotaoAnterior = LOW;
bool atualizarLeds = false; // Indica se os LEDs devem ser atualizados

void setup() {
  pinMode(BOTAO, INPUT); // Configura o pino do botão como entrada
  fita.begin();
  fita.show(); // Inicializa os LEDs apagados
}

void loop() {
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

  delay(50); // Pequeno atraso para evitar leituras erradas do botão
}
