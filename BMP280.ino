//Incluindo Bibliotecas
#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; //I2C
float temperatura;
float pressao;
float altitude;

void setup() {
  //Iniciando a comunicação serial
  Serial.begin(9600);
  // Imprimindo Mensagem de teste no Monitor Serial
  Serial.println(F("BMP280 teste"));

  
  if (!bmp.begin(0x76)) { /*Definindo o endereço I2C como 0x76. Mudar, se necessário, para (0x77)*/
    
    //Imprime mensagem de erro no caso de endereço invalido ou não localizado. Modifique o valor 
    Serial.println(F(" Não foi possível encontrar um sensor BMP280 válido, verifique a fiação ou "
                      "tente outro endereço!"));
    while (1) delay(1000);
  }
}

void loop() {
  
    //Imprimindo os valores de Temperatura
    Serial.print(F("Temperatura = "));
    temperatura = bmp.readTemperature();
    Serial.print(temperatura);
    Serial.println(" °C");

    //Imprimindo os valores de Pressão
    Serial.print(F("Pressão = "));
    pressao = bmp.readPressure();
    Serial.print(pressao);
    Serial.println(" Pa");

    //Imprimindo os valores de Altitude Aproximada
    Serial.print(F("Altitude Aprox = "));
    altitude = bmp.readAltitude(pressao/100);
    Serial.print(altitude); /* Ajustar a pressão de nível do mar de acordo com o local! bmp.readAltitude(1013.25)*/
    Serial.println(" m");

    //Tempo de espera de 1 segundo
    Serial.println();
    delay(1000);
}