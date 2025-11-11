// Control de dos motores con puente H (L298N) y joystick HW-504
// Pines de motor (ajustar según conexión física)
#include <Arduino.h>

// Motor A
const uint8_t ENA_PIN = 6;  // PWM
const uint8_t IN1_PIN = 7;
const uint8_t IN2_PIN = 8;

// Motor B
const uint8_t IN3_PIN = 3;
const uint8_t IN4_PIN = 4;
const uint8_t ENB_PIN = 5;  // PWM

// Joystick HW-504
const uint8_t JOY_X_PIN = A0; // Eje X
const uint8_t JOY_Y_PIN = A1; // Eje Y
const uint8_t JOY_BTN_PIN = 2; // Pulsador del joystick (si está presente). Usar INPUT_PULLUP

// Parámetros
// Baud por defecto para el Monitor Serial. Cambia a 115200 si tu monitor usa esa velocidad.
const unsigned long SERIAL_BAUD = 9600;

// Centro del joystick (calibrado en setup)
int JOY_CENTER_X = 512;
int JOY_CENTER_Y = 512;
const int DEADZONE = 60; // zona muerta para evitar ruido
const uint8_t RAMP_STEP = 8; // cuánto puede cambiar la salida por ciclo (suavizado)
const int MIN_PWM = 50; // PWM mínimo para evitar zumbidos sin giro. Aumentar si sigue sin girar.

int prevLeft = 0;
int prevRight = 0;

// --- Nuevos parámetros para control de Freno y Modo Cruz ---
// Tiempo en milisegundos que el motor debe estar parado antes de invertir la dirección.
const unsigned long BRAKE_MS = 150; 

// Variables para gestionar el estado del freno
unsigned long leftBrakeStartTime = 0;
unsigned long rightBrakeStartTime = 0;
int lastLeftSpeed = 0;
int lastRightSpeed = 0;
// --- Fin de nuevos parámetros ---

// Botón: pulsación larga frena mientras se mantiene
int lastBtnState = HIGH;
unsigned long btnPressTime = 0;
const unsigned long LONG_PRESS_MS = 600;
const unsigned long DEBOUNCE_MS = 50;
unsigned long lastDebounceTime = 0;
bool emergencyStop = false;

// Convierte un valor [-255,255] en control de motor (dirección + PWM)
void setMotor(int pwmPin, int inPin1, int inPin2, int speed) {
  // speed: -255 .. 255
  if (speed > 0) {
    digitalWrite(inPin1, HIGH);
    digitalWrite(inPin2, LOW);
    // Mapear la velocidad para que el mínimo sea MIN_PWM
    int pwm = map(speed, 1, 255, MIN_PWM, 255);
    analogWrite(pwmPin, pwm);
  } else if (speed < 0) {
    digitalWrite(inPin1, LOW);
    digitalWrite(inPin2, HIGH);
    // Mapear la velocidad para que el mínimo sea MIN_PWM
    int pwm = map(-speed, 1, 255, MIN_PWM, 255);
    analogWrite(pwmPin, pwm);
  } else {
    // stop/coast: ambos LOW y PWM 0
    digitalWrite(inPin1, LOW);
    digitalWrite(inPin2, LOW);
    analogWrite(pwmPin, 0);
  }
}

// Aplica rampa suave desde prev hacia target
int ramp(int prev, int target) {
  int diff = target - prev;
  if (abs(diff) <= RAMP_STEP) return target;
  if (diff > 0) return prev + RAMP_STEP;
  return prev - RAMP_STEP;
}

void setup() {
  // Pines de motor
  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);

  pinMode(ENB_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);

  // Joystick
  pinMode(JOY_BTN_PIN, INPUT_PULLUP);

  // Inicializar salidas en apagado
  analogWrite(ENA_PIN, 0);
  analogWrite(ENB_PIN, 0);
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);

  // Serial.begin(115200);
  delay(100);
  Serial.begin(SERIAL_BAUD);
  delay(100);
  Serial.print("Control de 2 motores con joystick HW-504 - listo. Serial @ ");
  Serial.println(SERIAL_BAUD);

  // Pequeña calibración de centro (leer promedio rápido)
  long sumX = 0, sumY = 0;
  const int samples = 8;
  for (int i = 0; i < samples; ++i) {
    sumX += analogRead(JOY_X_PIN);
    sumY += analogRead(JOY_Y_PIN);
    delay(5);
  }
  // Si se desea, se podría guardar la calibración; por ahora solo muestro
  int avgX = sumX / samples;
  int avgY = sumY / samples;
  JOY_CENTER_X = avgX;
  JOY_CENTER_Y = avgY;
  Serial.print("Joy center approx X="); Serial.print(avgX);
  Serial.print(" Y="); Serial.println(avgY);
  Serial.println("If joystick behaves oddly, increase DEADZONE or recalibrate.");
}

void loop() {
  // Leer joystick
  int rawX = analogRead(JOY_X_PIN);
  int rawY = analogRead(JOY_Y_PIN);

  // Convertir a valores centrados (-512 a 511 aprox)
  int x = rawX - JOY_CENTER_X;
  int y = JOY_CENTER_Y - rawY; // Invertir Y: empujar hacia delante -> valor positivo

  // Aplicar zona muerta
  if (abs(x) < DEADZONE) x = 0;
  if (abs(y) < DEADZONE) y = 0;

  // Mapear los valores del joystick a la escala del motor (-255 a 255)
  int xs = map(x, -512, 512, -255, 255);
  int ys = map(y, -512, 512, -255, 255);

  int leftTarget = 0;
  int rightTarget = 0;

  // --- Lógica de "Modo Cruz" ---
  // Si el movimiento vertical (Y) es mayor que el horizontal (X), mover adelante/atrás
  if (abs(ys) > abs(xs)) {
    leftTarget = ys;
    rightTarget = ys;
  } 
  // Si el movimiento horizontal (X) es mayor, girar en el sitio
  else if (abs(xs) > abs(ys)) {
    leftTarget = xs;
    rightTarget = -xs;
  }
  // Si son iguales (o ambos cero), los targets se quedan en 0.

  // --- Gestión del botón (freno de emergencia) ---
  int btnReading = digitalRead(JOY_BTN_PIN);
  if (btnReading != lastBtnState) { lastDebounceTime = millis(); }
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (btnReading != lastBtnState) {
      if (btnReading == LOW) { btnPressTime = millis(); }
      lastBtnState = btnReading;
    }
  }
  if (lastBtnState == LOW && (millis() - btnPressTime) > LONG_PRESS_MS) {
    emergencyStop = true;
  } else {
    emergencyStop = false;
  }

  if (emergencyStop) {
    leftTarget = 0;
    rightTarget = 0;
  }

  // Aplicar rampa de aceleración
  int rampedLeft = ramp(prevLeft, leftTarget);
  int rampedRight = ramp(prevRight, rightTarget);
  prevLeft = rampedLeft;
  prevRight = rampedRight;

  // --- Lógica de Freno por Inversión de Giro ---
  // Motor Izquierdo
  if ((rampedLeft > 0 && lastLeftSpeed < 0) || (rampedLeft < 0 && lastLeftSpeed > 0)) {
    if (leftBrakeStartTime == 0) { leftBrakeStartTime = millis(); }
    if (millis() - leftBrakeStartTime < BRAKE_MS) {
      rampedLeft = 0; // Forzar freno
    } else {
      leftBrakeStartTime = 0; // Fin del freno, permitir movimiento
    }
  } else {
    leftBrakeStartTime = 0; // No hay cambio de dir, no frenar
  }

  // Motor Derecho
  if ((rampedRight > 0 && lastRightSpeed < 0) || (rampedRight < 0 && lastRightSpeed > 0)) {
    if (rightBrakeStartTime == 0) { rightBrakeStartTime = millis(); }
    if (millis() - rightBrakeStartTime < BRAKE_MS) {
      rampedRight = 0; // Forzar freno
    } else {
      rightBrakeStartTime = 0; // Fin del freno, permitir movimiento
    }
  } else {
    rightBrakeStartTime = 0; // No hay cambio de dir, no frenar
  }

  // Enviar comandos a los motores
  setMotor(ENA_PIN, IN1_PIN, IN2_PIN, rampedLeft);
  setMotor(ENB_PIN, IN3_PIN, IN4_PIN, rampedRight);

  // Guardar la última velocidad enviada a los motores
  lastLeftSpeed = rampedLeft;
  lastRightSpeed = rampedRight;

  // --- Debug Ocasional ---
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  if (now - lastPrint > 200) {
    Serial.print("L:"); Serial.print(rampedLeft);
    Serial.print(" R:"); Serial.print(rampedRight);
    if (leftBrakeStartTime > 0 || rightBrakeStartTime > 0) Serial.print(" [BRAKING]");
    if (emergencyStop) Serial.print(" [E-STOP]");
    Serial.print(" | Joy X:"); Serial.print(x);
    Serial.print(" Y:"); Serial.print(y);
    Serial.println();
    lastPrint = now;
  }

  delay(10);
}
