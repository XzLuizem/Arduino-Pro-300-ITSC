#include <Arduino.h>

int in1Pin = 8; // Pin digital para IN1
int in2Pin = 7; // Pin digital para IN2
int enaPin = 9; // Pin digital para ENA

void setup() {
  // put your setup code here, to run once:
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enaPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Avanzar
  digitalWrite(in1Pin, HIGH);
  digitalWrite(in2Pin, LOW);
  analogWrite(enaPin, 255); // Velocidad máxima
  delay(2000); // avanzar por 2 segundos

  // Detener
  digitalWrite(in1Pin, LOW);
  digitalWrite(in2Pin, LOW);
  delay(2000); // detener por 2 segundos

  // Retroceder
  digitalWrite(in1Pin, LOW);
  digitalWrite(in2Pin, HIGH);
  analogWrite(enaPin, 255); // Velocidad máxima
  delay(2000); // retroceder por 2 segundos

  // Detener
  digitalWrite(in1Pin, LOW);
  digitalWrite(in2Pin, LOW);
  delay(2000); // detener por 2 segundos  
}