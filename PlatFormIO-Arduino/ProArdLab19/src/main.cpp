#include <Arduino.h>

const int buzzerPin = 8; // Pin digital para el buzzer
const int ledPin = 13;   // Pin digital para el LED

void setup()
{
  Serial.begin(9600);         // Inicializa la comunicación serial
  pinMode(buzzerPin, OUTPUT); // Configura el pin digital como salida
  pinMode(ledPin, OUTPUT);    // Configura el pin digital como salida
}

void loop()
{
  if (Serial.available() > 0)
  {                             // Verifica si hay datos disponibles en el puerto serial
    tone(buzzerPin, 1000);      // Generar un tono de 1000Hz
    digitalWrite(ledPin, HIGH); // Encender el LED
    delay(500);                 // Mantener el tono por 0.5 segundos

    noTone(buzzerPin);         // Detener el tono
    digitalWrite(ledPin, LOW); // Apagar el LED
    delay(500);                // Esperar 0.5 segundos antes de generar el siguiente tono
  }
}