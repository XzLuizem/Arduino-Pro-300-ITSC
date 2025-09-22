#include <Arduino.h>

const int potPin = A0; // Pin analógico donde está el potenciómetro
const int buzzerPin = 9;  // Pin digital para el buzzer
const int maxThreshold = 1023; // valor máximo del potenciómetro

void setup() {
    pinMode(buzzerPin, OUTPUT); // Configurar el pin del buzzer como salida
    pinMode(potPin, INPUT); // Configurar el pin del potenciómetro como entrada
    Serial.begin(9600);
}

void loop() {
    // Leer el valor del potenciómetro
    int sensorValue = analogRead(potPin); // Lee el valor analógico del potenciómetro
    int buzzerFrequency = map(sensorValue, 0, maxThreshold, 100, 2000); // Mapear el valor a una frecuencia audible

    Serial.print("Valor del potenciómetro: ");
    Serial.println(sensorValue);
    Serial.print(" | Frecuencia del buzzer: ");
    Serial.println(buzzerFrequency);

    // Suena el buzzer si el valor leído alcanza el valor máximo
    if (sensorValue == maxThreshold) {
        tone(buzzerPin, 1000); // Produce un tono de 1000 Hz
        delay(1000); // Mantener el tono por 1 segundo
        noTone(buzzerPin); // Deja de producir el tono
    } else {
        noTone(buzzerPin); // No producir tono
    }

  delay(100); // Espera 100 milisegundos antes de leer de nuevo
}