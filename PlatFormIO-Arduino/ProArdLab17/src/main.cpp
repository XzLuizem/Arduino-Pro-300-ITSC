#include <Arduino.h>

const int relayPin = 3; // Pin dgital para el relé

void setup()
{
  pinMode(relayPin, OUTPUT); // Configura el pin como salida
}

void loop()
{
  digitalWrite(relayPin, HIGH); // Activar el relé (Encender el dispositivo de alta potencia)
  delay(5000);                  // Mantener activado 5 segundos
  digitalWrite(relayPin, LOW);  // Desactivar el relé (Apagar el dispositivo de alta potencia)
  delay(5000);                  // Mantener desactivado 5 segundos
}
