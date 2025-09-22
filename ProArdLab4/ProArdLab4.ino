int in1Pin = 8; // Pin digital para IN1
int in2Pin = 7; // Pin digital para IN2
int enaPin = 9; // Pin PWM para ENA

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
  delay(2000); // correr por 2 segundos

  // Detener
  digitalWrite(in1Pin, LOW);
  digitalWrite(in2Pin, LOW);
  analogWrite(enaPin, 0); // Detener el motor
  delay(2000); // Esperar 2 segundos

  // Retroceder
  digitalWrite(in1Pin, LOW);
  digitalWrite(in2Pin, HIGH);
  analogWrite(enaPin, 255); // Velocidad máxima
  delay(2000); // correr por 2 segundos

  // Detener
  digitalWrite(in1Pin, LOW);
  digitalWrite(in2Pin, LOW);
  analogWrite(enaPin, 0); // Detener el motor
  delay(2000); // Esperar 2 segundos
}
