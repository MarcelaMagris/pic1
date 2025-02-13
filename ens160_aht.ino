#include <Wire.h>              // Biblioteca para comunicação I2C
#include <Adafruit_AHTX0.h>   // Biblioteca para o sensor AHT20
#include "ScioSense_ENS160.h" // Biblioteca para o sensor ENS160

// Instância dos sensores
Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1); // Endereço padrão do ENS160

// Variáveis globais para armazenar as leituras
float temperatura; // Armazena a temperatura em °C
int umidade;       // Armazena a umidade relativa (%)
int eCO2;         // Armazena a concentração de CO2 equivalente (ppm)
int aqi;          // Índice de Qualidade do Ar
int tvoc;         // Composto Orgânico Volátil Total (ppb)

void setup() {
  Serial.begin(9600); // Inicializa a comunicação serial
  while (!Serial) {}  // Aguarda a inicialização do monitor serial

  // Inicializa o sensor AHT20 (temperatura e umidade)
  if (!aht.begin()) {
    Serial.println("Erro: AHT20 não encontrado!");
    while (1) delay(1000);
  }

  // Inicializa o sensor ENS160 (qualidade do ar)
  if (!ens160.begin()) {
    Serial.println("Erro: ENS160 não encontrado!");
    while (1) delay(1000);
  }

  // Configura o ENS160 no modo padrão de operação
  ens160.setMode(ENS160_OPMODE_STD);
}

void loop() {
  // Captura temperatura e umidade do AHT20
  sensors_event_t humidityEvent, tempEvent;
  aht.getEvent(&humidityEvent, &tempEvent);
  temperatura = tempEvent.temperature;
  umidade = humidityEvent.relative_humidity;

  // Envia os valores de temperatura e umidade para o ENS160
  ens160.set_envdata(temperatura, umidade);

  // Mede os dados de qualidade do ar
  ens160.measure(true);

  // Captura os valores do ENS160
  aqi = ens160.getAQI();
  tvoc = ens160.getTVOC();
  eCO2 = ens160.geteCO2();

  // Exibe os dados no monitor serial
  Serial.print("Temperatura: "); Serial.print(temperatura); Serial.println(" °C");
  Serial.print("Umidade: "); Serial.print(umidade); Serial.println(" %");
  Serial.print("AQI: "); Serial.print(aqi); Serial.println();
  Serial.print("TVOC: "); Serial.print(tvoc); Serial.println(" ppb");
  Serial.print("eCO2: "); Serial.print(eCO2); Serial.println(" ppm");
  Serial.println("--------------------------");

  delay(5000); // Aguarda 5 segundos antes da próxima leitura
}
