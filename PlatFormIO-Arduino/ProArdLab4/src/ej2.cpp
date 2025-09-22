#include <Arduino.h>

// Definir los pines para el controlador del motor L298N
const int in1Pin = 8; // Pin digital para IN1
const int in2Pin = 7; // Pin digital para IN2  
const int enaPin = 9; // Pin digital para ENA

// Definir el pin digital para el botón
const int buttonPin = 2; // Pin digital para el botón

void setup() {
    // Configurar los pines del controlador del motor L298N como salidas
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(enaPin, OUTPUT);

    // Configurar el pin del botón como entrada
    pinMode(buttonPin, INPUT_PULLUP);

    // Inicializar el motor detenido
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enaPin, 0);
}

void loop() {
    // Leer el estado del botón
    int buttonState = digitalRead(buttonPin);

    // Si el botón está presionado(0), encender el motor
    if (buttonState == LOW) {
        digitalWrite(in1Pin, HIGH);
        digitalWrite(in2Pin, LOW);
        analogWrite(enaPin, 255); // Velocidad máxima
    } else {
        // Si el botón no está presionado, detener el motor
        digitalWrite(in1Pin, LOW);
        digitalWrite(in2Pin, LOW);
        analogWrite(enaPin, 0);
    }
}