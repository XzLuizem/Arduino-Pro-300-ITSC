#include <Arduino.h>

const int triggerPin = 9;
const int echoPin = 10;

// Speed of sound in cm/microsecond
const float SOUND_SPEED = 0.034;

const float MIN_DISTANCE_CM = 4.0;

void setup()
{
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
}

void loop()
{
  // Trigger the sensor by sending a short pulse
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // Read the echo pulse duration
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  // The sound wave travels to the object and back, so we divide by 2
  float distance = (duration * SOUND_SPEED) / 2;

  if (distance > 0 && distance < MIN_DISTANCE_CM) {
    Serial.print("Distancia: ");
    Serial.print(distance);
    Serial.println(" cm (Objeto demasiado cerca)");
  } else if (distance >= MIN_DISTANCE_CM) {
    Serial.print("Distancia: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("ADVERTENCIA: Lectura inválida. Fuera del rango del sensor.");
  }

  delay(1000);
}