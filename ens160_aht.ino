#include <Wire.h>  // Biblioteca para comunicação I2C

int ArduinoLED = 2; // Pino do LED embutido no Arduino

///// Início da configuração do AHT20
#include <Adafruit_AHTX0.h>  // Biblioteca do sensor de temperatura e umidade AHT20

Adafruit_AHTX0 aht;  // Objeto para comunicação com o sensor AHT20

// Variáveis para armazenar as leituras do sensor AHT20
float temperatura;  // Armazena a temperatura em graus Celsius
int umidade;        // Armazena a umidade relativa do ar

///// Fim da configuração do AHT20

#include "ScioSense_ENS160.h"  // Biblioteca do sensor de qualidade do ar ENS160
ScioSense_ENS160 ens160(ENS160_I2CADDR_1); // Objeto para comunicação com o sensor ENS160

// Variáveis para armazenar os dados do ENS160
int aqi;   // Índice de qualidade do ar (Air Quality Index)
int tvoc;  // Compostos orgânicos voláteis totais (Total Volatile Organic Compounds) em ppb
int eCO2;  // Nível estimado de CO2 equivalente em ppm

/*--------------------------------------------------------------------------
  FUNÇÃO SETUP
  Configura os sensores e inicializa a comunicação
 --------------------------------------------------------------------------*/
void setup() {
  Serial.begin(9600); // Inicia a comunicação serial a 9600 bps

  while (!Serial) {} // Aguarda até que a comunicação serial esteja disponível

  // Configura o LED embutido como saída e o mantém apagado
  pinMode(ArduinoLED, OUTPUT);
  digitalWrite(ArduinoLED, LOW);

  // Mensagens de inicialização no monitor serial
  Serial.println("------------------------------------------------------------");
  Serial.println("ENS160 - Sensor digital de qualidade do ar");
  Serial.println();
  Serial.println("Leitura do sensor no modo padrão");
  Serial.println();
  Serial.println("------------------------------------------------------------");
  delay(1000);

  Serial.print("Inicializando ENS160...");
  ens160.begin(); // Inicializa o sensor ENS160
  Serial.println(ens160.available() ? "concluído." : "falhou!");

  if (ens160.available()) {
    // Exibe as versões do firmware do ENS160
    Serial.print("\tVersão: "); Serial.print(ens160.getMajorRev());
    Serial.print("."); Serial.print(ens160.getMinorRev());
    Serial.print("."); Serial.println(ens160.getBuild());
  
    // Configura o ENS160 no modo padrão de operação
    Serial.print("\tModo padrão ");
    Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "ativado." : "falhou!");
  }

  // Inicialização do sensor AHT20
  Serial.println("Inicializando AHT10/AHT20...");
  if (!aht.begin()) {
    Serial.println("Não foi possível encontrar o AHT! Verifique a fiação.");
    while (1) delay(1000); // Loop infinito caso o sensor não seja encontrado
  }
  Serial.println("Sensor AHT10 ou AHT20 detectado com sucesso.");
} // Fim da função setup()

/*--------------------------------------------------------------------------
  FUNÇÃO LOOP PRINCIPAL
  Executa medições a cada 1000ms
 --------------------------------------------------------------------------*/
void loop() {
  // Captura os dados do sensor AHT20
  sensors_event_t humidity1, temp;
  aht.getEvent(&humidity1, &temp); // Atualiza os valores de temperatura e umidade
  temperatura = temp.temperature;  // Armazena a temperatura em graus Celsius
  umidade = humidity1.relative_humidity; // Armazena a umidade relativa

  // Exibe os valores lidos do AHT20 no monitor serial
  Serial.print("Temperatura: "); 
  Serial.print(temperatura); 
  Serial.println(" °C");
  Serial.print("Umidade: "); 
  Serial.print(umidade); 
  Serial.println("% rH");

  delay(1000); // Aguarda 1 segundo antes da próxima leitura

  // Verifica se o ENS160 está disponível
  if (ens160.available()) {
    // Envia os valores de temperatura e umidade para o sensor ENS160
    ens160.set_envdata(temperatura, umidade);

    // Realiza medições
    ens160.measure(true);
    ens160.measureRaw(true);

    // Captura os valores do sensor ENS160
    aqi = ens160.getAQI();    // Índice de qualidade do ar -- 1 para muito bom, 5 para ruim--
    tvoc = ens160.getTVOC();  // TVOC em ppb
    eCO2 = ens160.geteCO2();  // eCO2 em ppm

    // Exibe os valores do ENS160 no monitor serial
    Serial.print("AQI: "); Serial.print(aqi); Serial.print("\t");
    Serial.print("TVOC: "); Serial.print(tvoc); Serial.print(" ppb\t");
    Serial.print("eCO2: "); Serial.print(eCO2); Serial.println(" ppm");
  }
  delay(1000); // Aguarda 1 segundo antes da próxima leitura
} // Fim da função loop()
