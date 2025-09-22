#include <Arduino.h>

// Definir los pines para el controlador del motor L298N
const int in1Pin = 8; // Pin digital para IN1
const int in2Pin = 7; // Pin digital para IN2
const int enaPin = 9; // Pin digital para ENA

// Definir el pin analógico para el potenciómetro
const int potPin = A0; // Pin analógico para el potenciómetro

void setup() {
    // Configurar los pines del controlador del motor L298N como salidas
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(enaPin, OUTPUT);

    // Configurar el pin del potenciómetro como entrada
    pinMode(potPin, INPUT);

    // Inicializar el motor detenido
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enaPin, 0);
}

void loop() {
    // Leer el valor del potenciómetro (0-1023)
    int potValue = analogRead(potPin);

    // Mapear el valor del potenciómetro a un rango de 0-255 para PWM
    int pwmValue = map(potValue, 0, 1023, 0, 255);

    // Controlar la velocidad del motor
    analogWrite(enaPin, pwmValue);

    // Girar el motor en una dirección
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    
    // Esperar un segundo antes de la siguiente lectura
    delay(1000);
}