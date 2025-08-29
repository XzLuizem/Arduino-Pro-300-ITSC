int led = 13;
int valor;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    valor = Serial.read();
  }
  if (valor == 'a') {
    digitalWrite(led, HIGH);
  }
  if (valor == 'b') {
    digitalWrite(led, LOW);
  }
}
