#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Iniciar la comunicación en serie
}

void loop() {
  // put your main code here, to run repeatedly:
  int sensorValue = analogRead(A0); // Leer el valor analogico del potenciómetro
  int mappedValue = map(sensorValue, 0, 1023, 0, 100); // Mapear el valor a un rango de 0 a 100

  Serial.print("Valor del potenciómetro: "); // Imprimir el valor original
  Serial.print(sensorValue);
  Serial.print(" | Valor mapeado: "); // Imprimir el valor mapeado
  Serial.println(mappedValue);

  delay(1000); // Esperar 1 segundo antes de la siguiente lectura
}