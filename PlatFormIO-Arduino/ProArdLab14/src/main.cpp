#include <Arduino.h>

const int pirPin = 2; // Pin digital para el sensor PIR
const int buzzerPin = 3; // Pin digital para el buzzer
const int ledPin = 13; // Pin digital para el LED

void setup()
{
  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  int pirState = digitalRead(pirPin); // Lee el estado del sensor PIR

  if (pirState == HIGH) // Si el sensor detecta movimiento
  {
    Serial.println("Movimiento detectado!");
    // Hacer parpadear el LED y buzzer 3 veces
    for(int i = 0; i < 3; i++) {
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(ledPin, HIGH);
      delay(200); // Encendido por 200ms
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);
      delay(200); // Apagado por 200ms
    }
    delay(1000); // Pausa antes de volver a detectar
  }
  else
  {
    digitalWrite(buzzerPin, LOW); // Mantener el buzzer apagado
    digitalWrite(ledPin, LOW); // Mantener el LED apagado
  }
  delay(100);
}