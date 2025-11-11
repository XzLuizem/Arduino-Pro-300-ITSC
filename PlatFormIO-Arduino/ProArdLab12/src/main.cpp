#include <Arduino.h>

const int potPin = A0;   // Pin analógico para el potenciómetro

void setup() {
    pinMode(9, OUTPUT);   // Rojo
    pinMode(10, OUTPUT);  // Verde
    pinMode(11, OUTPUT);  // Azul
    pinMode(potPin, INPUT);
}

void loop() {
    int potValue = analogRead(potPin);          // 0‑1023
    float hue = map(potValue, 0, 1023, 0, 360); // 0‑360 grados

    // Convertir HSV a RGB (valor de saturación y brillo al 100%)
    float s = 1.0;
    float v = 1.0;
    float c = v * s;
    float x = c * (1 - abs(fmod(hue / 60.0, 2) - 1));
    float m = v - c;

    float r1, g1, b1;
    if (hue < 60)      { r1 = c; g1 = x; b1 = 0; }
    else if (hue < 120) { r1 = x; g1 = c; b1 = 0; }
    else if (hue < 180) { r1 = 0; g1 = c; b1 = x; }
    else if (hue < 240) { r1 = 0; g1 = x; b1 = c; }
    else if (hue < 300) { r1 = x; g1 = 0; b1 = c; }
    else                { r1 = c; g1 = 0; b1 = x; }

    int r = (int)((r1 + m) * 255);
    int g = (int)((g1 + m) * 255);
    int b = (int)((b1 + m) * 255);

    analogWrite(9, r);   // Rojo
    analogWrite(10, g);  // Verde
    analogWrite(11, b);  // Azul

    delay(50);           // Ajusta la velocidad de cambio
}