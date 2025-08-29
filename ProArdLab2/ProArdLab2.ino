const int buttonPin = 3;  // Pin digital para el pulsador
const int ledPin = 10;    // Pin digital para el LED

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);    // Configura el pin del LED como salida
  pinMode(buttonPin, INPUT);  // Configura el pin del pulsador como entrada
  digitalWrite(ledPin, LOW);  // Inicializa el LED apagado
}

void loop() {
  // Comprueba si el pulsador ha sido presionado
  if (digitalRead(buttonPin) == LOW) {
    digitalWrite(ledPin, HIGH);
    while (digitalRead(buttonPin) == LOW) {
      // EL LED permanecera encendido
    }
    digitalWrite(ledPin, LOW);  // Apaga el LED
  }
}