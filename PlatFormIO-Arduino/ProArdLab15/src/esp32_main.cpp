/*
  ESP32 ECG Monitor with AD8232 and LCD Display

  Conexiones AD8232:
  - VCC → 3.3V (ESP32)
  - GND → GND  (ESP32)
  - OUTPUT → GPIO34 (ADC1_CH6)
  - LO- → GPIO12
  - LO+ → GPIO13

  Nota: Los pines LO+ y LO- detectan si los electrodos están conectados.
        HIGH = electrodo desconectado
        LOW  = electrodo conectado correctamente

  Conexiones LCD I2C:
  - SDA → GPIO21
  - SCL → GPIO22
  - VCC → 5V
  - GND → GND
*/

#include <Arduino.h>
#include "ecg_types.h"
#include "lcd_manager.h"
#include "ecg_monitor.h"

// Objetos globales
LCDManager lcd;
ECGMonitor ecgMonitor;

// Habilitar salida CSV para plotter (habilitar para usar tools/plot_ecg.py)
#define ENABLE_CSV_OUTPUT 1

// Variables para el timer
volatile bool sampleFlag = false;
hw_timer_t *timer = NULL;

// Variable para control de actualización de LCD
unsigned long lastLCDUpdate = 0;
const unsigned long LCD_UPDATE_INTERVAL = 100; // Actualizar LCD cada 100ms

void IRAM_ATTR onTimer()
{
  // set flag only, keep ISR minimal
  sampleFlag = true;
}

void setup()
{
  Serial.begin(115200);

  // Inicializar LCD
  lcd.begin();
  // Mostrar dirección I2C detectada
  Serial.print("LCD I2C address: 0x");
  Serial.println(lcd.getAddress(), HEX);

  // Inicializar monitor ECG
  ecgMonitor.begin();

  // Mostrar mensaje inicial sobre electrodos
  lcd.updateStatus("Esperando...");

  // Configurar timer para muestreo
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, SAMPLE_INTERVAL_US, true);
  timerAlarmEnable(timer);

  lcd.updateStatus("Listo");
}

// Helper function to send data in a unified format for the plotter
void sendPlotterData(float filteredValue, float bpm, bool leadsConnected)
{
  static char buffer[50];
  int leadStatus = leadsConnected ? 1 : 0;

  // Unified CSV format: filtered_value,bpm,lead_status
  snprintf(buffer, sizeof(buffer), "%.2f,%d,%d", filteredValue, (int)bpm, leadStatus);
  
  Serial.println(buffer);
}

void loop()
{
  if (sampleFlag)
  {
    sampleFlag = false;

    // Procesar nueva muestra
    bool beatDetected = ecgMonitor.processSample();
    bool leadsAreConnected = ecgMonitor.checkLeadsConnected();

    // Enviar datos al plotter de Python si los electrodos están conectados
    if (leadsAreConnected) {
      sendPlotterData(
          ecgMonitor.getLastFilteredValue(),
          ecgMonitor.getBeatInfo().bpm,
          leadsAreConnected
      );
    }

    // Tareas de actualización del LCD (se ejecutan con menos frecuencia para evitar parpadeo)
    unsigned long now = millis();
    if (now - lastLCDUpdate >= LCD_UPDATE_INTERVAL)
    {
      lastLCDUpdate = now;

      // Gestionar el apagado del indicador de latido (corazón)
      lcd.manageBeatIndicator();

      // Actualizar la barra de señal a una velocidad visualmente agradable
      lcd.updateSignalBar(ecgMonitor.getLastFilteredValue());

      // Actualizar Status y BPM
      if (!leadsAreConnected)
      {
        lcd.updateStatus("Sin electrodos");
        lcd.updateBPM(0);
      }
      else
      {
        lcd.updateStatus("Monitoreando");
        const BeatInfo &beatInfo = ecgMonitor.getBeatInfo();
        lcd.updateBPM(beatInfo.bpm);
      }
    }

    // Mostrar el indicador de latido en el LCD instantáneamente (no bloqueante)
    if (beatDetected && leadsAreConnected)
    {
      lcd.showBeat();
    }
  }
}
