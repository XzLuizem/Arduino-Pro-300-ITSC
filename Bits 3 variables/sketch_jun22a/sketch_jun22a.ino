int Led = 13, A = 2, B = 3, C = 4, bit;
String a = "LOW", b = "LOW", c = "LOW";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(Led, OUTPUT);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(Led, HIGH);
  delay(100);
  digitalWrite(Led, LOW);
  delay(1500);
  if (Serial.available() > 0) {
    bit = Serial.read();
    switch (bit) {
      case '0':
        digitalWrite(A, LOW);
        digitalWrite(B, LOW);
        digitalWrite(C, LOW);
        a = "LOW";
        b = "LOW";
        c = "LOW";
        break;
      case '1':
        if (a == "LOW") {
          digitalWrite(A, HIGH);
          a = "HIGH";
        } else {
          digitalWrite(A, LOW);
          a = "LOW";
        }
        break;
      case '2':
        if (b == "LOW") {
          digitalWrite(B, HIGH);
          b = "HIGH";
        } else {
          digitalWrite(B, LOW);
          b = "LOW";
        }
        break;
      case '3':
        if (C == "LOW") {
          digitalWrite(C, HIGH);
          c = "HIGH";
        } else {
          digitalWrite(C, LOW);
          c = "LOW";
        }
        break;
    }
  }
}
