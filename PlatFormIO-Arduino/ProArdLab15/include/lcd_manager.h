/**
 * @file lcd_manager.h
 * @brief Gestor de la pantalla LCD I2C, optimizado para 20x4.
 */

#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ecg_types.h"

// Definición del caracter personalizado de corazón
byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

class LCDManager {
public:
    LCDManager() : lcd(nullptr), currentAddr(LCDPins::ADDR), beatDisplayTime(0) {}

    void begin() {
        Wire.begin(LCDPins::SDA, LCDPins::SCL);
        currentAddr = scanForLCD();

        if (lcd) delete lcd;
        lcd = new LiquidCrystal_I2C(currentAddr, 20, 4);
        lcd->init();
        lcd->backlight();
        lcd->clear();

        lcd->createChar(0, heart);

        lcd->setCursor(0, 0);
        lcd->print("Monitor ECG AD8232");
        lcd->setCursor(0, 1);
        lcd->print("BPM: ---");
        lcd->setCursor(0, 2);
        lcd->print("Status: Iniciando...");
        lcd->setCursor(0, 3);
        lcd->print("Signal: [..........]");
    }

    uint8_t getAddress() const { return currentAddr; }

    void updateBPM(float bpm) {
        if (!lcd) return;
        lcd->setCursor(5, 1);
        if (bpm > 0) {
            char bpmStr[4];
            sprintf(bpmStr, "%3.0f", bpm);
            lcd->print(bpmStr);
        } else {
            lcd->print("---");
        }
    }

    void updateStatus(const char* status) {
        if (!lcd) return;
        lcd->setCursor(8, 2);
        lcd->print("            ");
        lcd->setCursor(8, 2);
        lcd->print(status);
    }

    // Muestra el corazón y registra el tiempo (no bloqueante)
    void showBeat() {
        if (!lcd) return;
        lcd->setCursor(16, 1);
        lcd->write((uint8_t)0);
        beatDisplayTime = millis();
    }

    // Gestiona el apagado del corazón después de un tiempo
    void manageBeatIndicator() {
        if (beatDisplayTime > 0 && millis() - beatDisplayTime > BEAT_DISPLAY_DURATION) {
            lcd->setCursor(16, 1);
            lcd->print(" ");
            beatDisplayTime = 0; // Resetea el temporizador
        }
    }

    void updateSignalBar(int filteredValue) {
        if (!lcd) return;
        // Mapear el valor filtrado a una escala de 0-18 para la barra
        // Un rango de -400 a 400 suele ser un buen punto de partida para la visualización
        int mappedValue = map(constrain(filteredValue, -400, 400), -400, 400, 0, 18);
        
        lcd->setCursor(8, 3);
        lcd->print("[");
        for (int i = 0; i < 10; ++i) {
            lcd->print(i < mappedValue ? "=" : ".");
        }
        lcd->print("]");
    }

private:
    LiquidCrystal_I2C* lcd;
    uint8_t currentAddr;
    unsigned long beatDisplayTime;
    static const unsigned long BEAT_DISPLAY_DURATION = 100; // ms

    uint8_t scanForLCD() {
        uint8_t found = 0;
        for (uint8_t addr = 1; addr < 127; addr++) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                if (addr == 0x27 || addr == 0x3F) return addr;
                if (found == 0) found = addr;
            }
        }
        return (found != 0) ? found : LCDPins::ADDR;
    }
};

#endif // LCD_MANAGER_H