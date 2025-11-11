#include <Arduino.h>
#include <HC_SR04.h>

// Define los pines para mayor claridad
// const int TRIGGER_PIN = 9;
// const int ECHO_PIN = 10;

HC_SR04<9> ultrasonicSensor(10);

void setup()
{
  Serial.begin(9600);
  // Pequeño retraso para dar tiempo a que el monitor serie se conecte
  delay(1000);
  Serial.println("Iniciando prueba del sensor HC-SR04...");
  ultrasonicSensor.begin();
}

void loop()
{
  float distance = ultrasonicSensor.getDist_cm();

  Serial.print("Lectura del sensor: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Comprueba valores de error comunes (0 o menos)
  if (distance <= 0)
  {
    Serial.println("ADVERTENCIA: Lectura inválida. Verifica el cableado o si hay un objeto muy cerca.");
  }

  delay(1000);
}