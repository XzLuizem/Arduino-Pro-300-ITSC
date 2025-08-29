void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);  // configura el pin 13 como salida
  pinMode(12, OUTPUT);  // configura el pin 12 como salida
  pinMode(11, OUTPUT);  // configura el pin 11 como salida
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, HIGH);  // Enciende el LED en el pin 13
  digitalWrite(12, LOW);   // Apaga el LED en el pin 12
  digitalWrite(11, LOW);   // Apaga el LED en el pin 11
  delay(200);              // Espera 200 milisegundos
  digitalWrite(13, LOW);   // Apaga el LED en el pin 13
  digitalWrite(12, HIGH);  // Enciende el LED en el pin 12
  digitalWrite(11, LOW);   // Apaga el LED en el pin 11
  delay(200);              // Espera 200 milisegundos
  digitalWrite(13, LOW);   // Apaga el LED en el pin 13
  digitalWrite(12, LOW);   // Apaga el LED en el pin 12
  digitalWrite(11, HIGH);  // Enciende el LED en el pin 11
  delay(200);              // Espera 200 milisegundos
}
