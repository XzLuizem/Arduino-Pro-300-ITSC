#include <Arduino.h>

// Pines
const int pirPin = 2;    // Pin digital para el sensor PIR
const int buzzerPin = 3; // Pin digital para el buzzer activo TMB12A05
const int ledPin = 13;   // Pin digital para el LED

// Configuración del buzzer activo
// El TMB12A05 es un buzzer activo que suena cuando se le aplica HIGH en el pin.
const bool BUZZER_ACTIVE_HIGH = true;
const bool USE_TONE = false; // no usar tone() para buzzer activo

// Tiempos (ms)
const unsigned long STARTUP_IGNORE_MS = 30000; // Tiempo de calibración del PIR (30s)
const unsigned long BUZZER_ON_MS = 500;        // Duración del pitido al detectar movimiento
const unsigned long LED_HOLD_MS = 2000;        // Mantiene el LED encendido tras la detección

// Estado
unsigned long startupMillis = 0;
unsigned long lastMotionMillis = 0;
unsigned long buzzerOnMillis = 0;
bool buzzerActive = false;
bool ledActive = false;
bool startupMessageShown = false;
int prevPirState = LOW; // para detectar flancos y enviar solo un mensaje por evento

void setBuzzer(bool on)
{
  // Buzzer activo: usar digitalWrite
  digitalWrite(buzzerPin, (on == BUZZER_ACTIVE_HIGH) ? HIGH : LOW);
}

void setup()
{
  // Intentar fijar el nivel "apagado" antes de poner el pin como salida.
  digitalWrite(buzzerPin, BUZZER_ACTIVE_HIGH ? LOW : HIGH);
  pinMode(buzzerPin, OUTPUT);

  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  startupMillis = millis();
}

void loop()
{
  unsigned long now = millis();

  // Durante la calibración del PIR (arranque) ignoramos eventos
  if (now - startupMillis < STARTUP_IGNORE_MS)
  {
    if (!startupMessageShown)
    {
      Serial.println("Calibrando sensor PIR. Ignorando eventos por unos segundos...");
      startupMessageShown = true;
    }
    setBuzzer(false);
    digitalWrite(ledPin, LOW);
    delay(100);
    return;
  }

  int pirState = digitalRead(pirPin);

  // Flanco ascendente -> nuevo evento
  if (pirState == HIGH && prevPirState == LOW)
  {
    Serial.println("Movimiento detectado!");
    lastMotionMillis = now;
    if (!buzzerActive)
    {
      setBuzzer(true);
      buzzerOnMillis = now;
      buzzerActive = true;
    }
    ledActive = true;
  }

  prevPirState = pirState;

  // Apagar buzzer tras la duración configurada
  if (buzzerActive && (now - buzzerOnMillis >= BUZZER_ON_MS))
  {
    setBuzzer(false);
    buzzerActive = false;
  }

  // Mantener LED encendido por LED_HOLD_MS desde la última detección
  if (ledActive)
  {
    digitalWrite(ledPin, HIGH);
    if (now - lastMotionMillis >= LED_HOLD_MS)
    {
      ledActive = false;
      digitalWrite(ledPin, LOW);
    }
  }

  delay(50);
}
