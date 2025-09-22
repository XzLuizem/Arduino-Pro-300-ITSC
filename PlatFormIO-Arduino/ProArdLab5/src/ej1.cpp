#include <Arduino.h>

const int potPin = A0; // Pin analógico donde está el potenciómetro
const int ledPin = 13;  // Pin digital para el LED
const int threshold = 512; // Umbral para encender el LED (ajustable)

void setup() {
    pinMode(ledPin, OUTPUT); // Configurar el pin del LED como salida
    pinMode(potPin, INPUT); // Configurar el pin del potenciómetro como entrada
    Serial.begin(9600);
}

void loop() {
    // Leer el valor del potenciómetro
    int sensorValue = analogRead(potPin); // Lee el valor analógico del potenciómetro
    int ledState = map(sensorValue, 0, 1023, 0, 1); // Mapear el valor a 0 o 1

    Serial.print("Valor del potenciómetro: ");
    Serial.print(sensorValue);
    Serial.print(" | Estado del LED: ");
    Serial.println(ledState);

    if (sensorValue > threshold) {
        digitalWrite(ledPin, HIGH); // Encender el LED
    } else {
        digitalWrite(ledPin, LOW); // Apagar el LED
    }

  delay(100); // Pequeña pausa para estabilidad
}