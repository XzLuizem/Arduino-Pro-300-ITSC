/*
  Código optimizado para sensor MQ-2
  - Lectura directa del ADC
  - Suavizado por promedio de muestras
  - Cálculo de Rs y PPM usando la relación: PPM = A * (Rs/R0)^B
  - Comando por Serial: 'C' -> calibrar R0 en el aire actual (medido directamente)
  Nota: Ajusta RL_KOHM, R0_KOHM inicial y constantes A/B según tu calibración o la hoja/datos del sensor.
*/

#include <Arduino.h>
#include <EEPROM.h>

// Pines y parámetros
const uint8_t MQ2_PIN = A0;
const float ADC_MAX = 1023.0; // ADC 10-bit (Arduino UNO/AVR)
const float VCC = 5.0; // tensión de referencia (ajusta si usas 3.3V)

// Persistencia en EEPROM
const int EEPROM_R0_ADDR = 0;  // dirección para almacenar R0
const float R0_DEFAULT = 10.0;  // valor por defecto si EEPROM está vacía
const uint32_t EEPROM_MAGIC = 0xAB12CD34; // marca para detectar si hay datos válidos
const int EEPROM_MAGIC_ADDR = sizeof(float); // después de R0

// Valores del circuito (en kilo-ohmios para consistencia)
const float RL_KOHM = 10.0; // resistencia de carga RL en kilo-ohmios
float R0_KOHM = R0_DEFAULT; // se actualiza desde EEPROM en setup()

// Curva empírica: PPM = A * (Rs/R0)^B
// Los valores A/B provienen de curvas típicas (puedes ajustarlos tras calibrar)
const float A_CURVE = 110.47;   // A
const float B_CURVE = -2.862;   // B

// Muestreo / suavizado
const uint16_t SAMPLES = 6;      // número de lecturas por muestra (promedio)
const uint16_t SAMPLE_INTERVAL_MS = 50; // intervalo entre muestras
const uint16_t LOOP_DELAY_MS = 1000;    // retardo entre lecturas en loop

// Alarma: LED y Buzzer
const uint8_t LED_PIN = 13;        // LED indicadora (usa 13 o cambia a otro pin)
const uint8_t BUZZER_PIN = 8;      // Buzzer activo — usa pin digital
const float PPM_THRESHOLD = 50.0;  // umbral PPM para activar alarma
                                  // Basado en las lecturas observadas:
                                  // - Aire limpio: ~100-120 PPM
                                  // - Detección gas: PPM cae por debajo de 50
const float PPM_HYSTERESIS = 0.90; // umbral de salida = threshold * hysteresis (evita parpadeo)

// Estado de alarma
static bool alarmActive = false;
// Parpadeo (no bloqueante)
const uint16_t BLINK_ON_MS = 300;
const uint16_t BLINK_OFF_MS = 300;
static unsigned long lastBlinkMillis = 0;
static bool blinkState = false;
static bool prevAlarmState = false;


// Funciones auxiliares
static float readAverageADC(uint16_t samples, uint16_t delayMs) {
  unsigned long sum = 0;
  for (uint16_t i = 0; i < samples; ++i) {
    int val = analogRead(MQ2_PIN);
    sum += val;
    delay(delayMs);
  }
  return (float)sum / (float)samples;
}

// Calcula la resistencia del sensor Rs (en las mismas unidades kOhm que RL_KOHM)
static float rawToRs(float rawADC)
{
  if (rawADC <= 0.0f) return INFINITY; // protección división por cero
  if (rawADC >= ADC_MAX) return 0.0f;   // saturación
  // Topología: Vcc -- RL -- Vout -- Rs -- GND
  // Vout = Vcc * (Rs / (Rs + RL)) => Rs = RL * (Vout / (Vcc - Vout))
  // Con lecturas ADC: Vout/Vcc = rawADC/ADC_MAX
  // Rs = RL * ( rawADC / (ADC_MAX - rawADC) )
  return RL_KOHM * ( rawADC / (ADC_MAX - rawADC) );
}

// Calculo de ppm usando curva empírica
static float rsToPpm(float rs_kohm, float r0_kohm)
{
  if (!isfinite(rs_kohm) || r0_kohm <= 0.0f) return -1.0f;
  float ratio = rs_kohm / r0_kohm;
  // PPM = A * ratio ^ B
  float ppm = A_CURVE * pow(ratio, B_CURVE);
  return ppm;
}

// Calibración simple: toma N muestras y asigna R0 = Rs_promedio
// (Colocar sensor en aire limpio antes de pulsar 'C' en el monitor serie)
static float calibrateR0(uint16_t samples, uint16_t sampleIntervalMs)
{
  float adc = readAverageADC(samples, sampleIntervalMs);
  float rs = rawToRs(adc);
  // Aquí guardamos R0 igual a la resistencia medida en aire limpio.
  // Alternativamente, si conoces el factor limpio (Rs/R0 en aire limpio), usa: R0 = Rs / factor
  return rs;
}

// Guardar R0 en EEPROM
static void saveR0ToEEPROM(float r0) {
  EEPROM.put(EEPROM_R0_ADDR, r0);
  EEPROM.put(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
}

// Cargar R0 desde EEPROM
static float loadR0FromEEPROM() {
  uint32_t magic;
  EEPROM.get(EEPROM_MAGIC_ADDR, magic);
  if (magic != EEPROM_MAGIC) {
    return R0_DEFAULT; // EEPROM sin inicializar
  }
  float stored_r0;
  EEPROM.get(EEPROM_R0_ADDR, stored_r0);
  return isfinite(stored_r0) && stored_r0 > 0.0f ? stored_r0 : R0_DEFAULT;
}

// Variables para fase de calentamiento
static const unsigned long WARMUP_TIME_MS = 10000; // 10 segundos de estabilización inicial
static unsigned long startupTime = 0;
static bool isWarmedUp = false;

void setup()
{
  Serial.begin(9600);  // velocidad más estable para monitores serie
  pinMode(MQ2_PIN, INPUT);
  // Pines de salida
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Asegurar que la alarma empieza desactivada
  alarmActive = false;
  blinkState = false;
  prevAlarmState = false;

  // Cargar R0 calibrado desde EEPROM
  R0_KOHM = loadR0FromEEPROM();
  
  // Iniciar temporizador de calentamiento
  startupTime = millis();

  Serial.println("\nMQ-2 sensor - lectura optimizada");
  Serial.print("R0 cargado desde EEPROM: ");
  Serial.print(R0_KOHM, 3);
  Serial.println(" kOhm");
  Serial.println("IMPORTANTE: Esperar precalentamiento (~3 min) antes de calibrar!");
  Serial.println("Comandos por Serial: 'C' -> calibra R0 (en aire limpio).\n");
}

void loop()
{
  // Control de fase de calentamiento
  if (!isWarmedUp) {
    if (millis() - startupTime < WARMUP_TIME_MS) {
      // Durante calentamiento, mantener todo apagado
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      Serial.println("Calentando sensor... espere");
      delay(1000);
      return;
    }
    isWarmedUp = true;
    Serial.println("\n*** Sensor listo para usar ***");
  }

  // Lectura promediada
  float adcAvg = readAverageADC(SAMPLES, SAMPLE_INTERVAL_MS);

  // Convertir a Rs (kOhm)
  float rs = rawToRs(adcAvg);

  // Calcular PPM
  float ppm = rsToPpm(rs, R0_KOHM);

  // Salida por Serial con formato legible
  Serial.print("ADC: ");
  Serial.print(adcAvg, 1);
  Serial.print("  Rs(kOhm): ");
  if (isfinite(rs)) Serial.print(rs, 3); else Serial.print("ERR");
  Serial.print("  R0(kOhm): ");
  Serial.print(R0_KOHM, 3);
  Serial.print("  PPM: ");
  if (ppm >= 0.0f && isfinite(ppm)) Serial.println(ppm, 2);
  else Serial.println("N/A");
  // Lógica de alarma con hysteresis y protección
  float upperThreshold = PPM_THRESHOLD / PPM_HYSTERESIS; // umbral superior para histéresis
  bool ppmValid = (ppm >= 0.0f && isfinite(ppm));

  if (!ppmValid) { // protección contra lecturas inválidas
    // Si la lectura es inválida, deshabilitar alarma
    alarmActive = false;
  } else if (ppm <= PPM_THRESHOLD) {
    // Activar alarma cuando PPM cae por debajo del umbral
    // (Rs disminuye en presencia de gas = PPM baja)
    alarmActive = true;
  } else if (ppm > upperThreshold) {
    // Desactivar alarma cuando PPM sube por encima del umbral superior
    alarmActive = false;
  }

  // Aplicar salidas (parpadeo no bloqueante cuando alarma activa)
  unsigned long now = millis();
  if (alarmActive) {
    // Cambiar estado de parpadeo según tiempos
    unsigned long interval = blinkState ? BLINK_ON_MS : BLINK_OFF_MS;
    if (now - lastBlinkMillis >= interval) {
      blinkState = !blinkState;
      lastBlinkMillis = now;
    }

    if (blinkState) {
      digitalWrite(LED_PIN, HIGH);
      // Buzzer activo: alimentamos el pin para producir sonido
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }
  } else {
    // Asegurar salidas apagadas y reiniciar parpadeo
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    blinkState = false;
    lastBlinkMillis = now;
  }

  // Imprimir mensaje sólo cuando cambia el estado de la alarma
  if (alarmActive != prevAlarmState) {
    if (alarmActive) Serial.println("*** ALARMA: PPM alto! LED y buzzer ACTIVADOS (parpadeo) ***");
    else Serial.println("Alarma desactivada.");
    prevAlarmState = alarmActive;
  }
  // Comprobar si hay datos por Serial para comandos
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'C' || c == 'c') {
      Serial.println("Iniciando calibracion: colocar MQ-2 en aire limpio y esperando...");
      float newR0 = calibrateR0(50, 50); // ~2.5 segundos
      if (isfinite(newR0) && newR0 > 0.0f) {
        R0_KOHM = newR0;
        saveR0ToEEPROM(R0_KOHM);
        Serial.print("Calibracion completada. Nuevo R0(kOhm): ");
        Serial.print(R0_KOHM, 3);
        Serial.println(" (guardado en EEPROM)");
      } else {
        Serial.println("Error durante calibracion.");
      }
    }
  }

  delay(LOOP_DELAY_MS);
}