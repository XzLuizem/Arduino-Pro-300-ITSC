#include <Arduino.h>

// Pines
const int pirPin = 2;    // Pin digital para el sensor PIR
const int buzzerPin = 3; // Pin digital para el buzzer (TMB12A05 es un buzzer activo)
const int ledPin = 13;   // Pin digital para el LED

// Configuración del buzzer
// Si tu buzzer suena cuando pones el pin HIGH, deja true.
// Si tu buzzer está cableado entre VCC y el pin (se activa con LOW), pon false.
// Para un buzzer pasivo (p. ej. HW508) usa USE_TONE = true y conecta el buzzer
// entre el pin y GND (low-side). El código generará la frecuencia con tone().
const bool BUZZER_ACTIVE_HIGH = true;
// Usar tone() para buzzer pasivo (HW508)
const bool USE_TONE = true;
const unsigned int TONE_FREQUENCY = 2000; // Hz recomendado para HW508, ajústalo si quieres

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

void setup()
{
  // Intentar fijar el nivel "apagado" antes de poner el pin como salida.
  // Hacer digitalWrite antes de pinMode(OUTPUT) garantiza que al cambiar a salida
  // el pin ya tenga el valor deseado y reduzca parpadeos.
  digitalWrite(buzzerPin, BUZZER_ACTIVE_HIGH ? LOW : HIGH);
  pinMode(buzzerPin, OUTPUT); // Configura el pin del buzzer como salida

  pinMode(pirPin, INPUT);  // Configura el pin del sensor PIR como entrada
  pinMode(ledPin, OUTPUT); // Configura el pin del LED como salida

  Serial.begin(9600); // Inicia la comunicación serial a 9600 baudios
  startupMillis = millis();
}

void setBuzzer(bool on)
{
  if (USE_TONE)
  {
    if (on)
      tone(buzzerPin, TONE_FREQUENCY);
    else
      noTone(buzzerPin);
  }
  else
  {
    digitalWrite(buzzerPin, (on == BUZZER_ACTIVE_HIGH) ? HIGH : LOW);
  }
}

void loop()
{
  unsigned long now = millis();

  // Durante la calibración del PIR (arranque) ignoramos eventos para evitar pitidos
  if (now - startupMillis < STARTUP_IGNORE_MS)
  {
    if (!startupMessageShown)
    {
      Serial.println("Calibrando sensor PIR. Ignorando eventos por unos segundos...");
      startupMessageShown = true;
    }
    // Nos aseguramos que buzzer y LED estén apagados
    setBuzzer(false);
    digitalWrite(ledPin, LOW);
    delay(100);
    return;
  }

  int pirState = digitalRead(pirPin); // Lee el estado del sensor PIR
  // Detectar flanco ascendente para enviar solo un mensaje por evento
  if (pirState == HIGH && prevPirState == LOW)
  {
    Serial.println("Movimiento detectado!");
    // Inicio de evento: actualizar tiempos y activar buzzer/LED
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

  // Apagar buzzer tras la duración configurada (no bloqueante)
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

  delay(50); // Pequeña pausa para evitar lecturas excesivas
}