/**
 * @file ecg_types.h
 * @brief Definiciones compartidas para el sistema ECG
 */

#ifndef ECG_TYPES_H
#define ECG_TYPES_H

#include <Arduino.h>

// Configuración de pines para AD8232 (conexiones a GPIO del ESP32)
// Nota: VCC y GND son las rails de alimentación (usar 3.3V y GND físicos)
struct AD8232Pins {
    // Pines de señal
    static constexpr int OUTPUT_PIN = 34;  // ADC1_CH6 - Señal analógica del ECG (OUTPUT)
    static constexpr int LO_PLUS = 13;     // Lead-Off Detection Positive (LO+)
    static constexpr int LO_MINUS = 12;    // Lead-Off Detection Negative (LO-)
};

// Configuración de pines para LCD I2C
struct LCDPins {
    static constexpr int SDA = 21;      // Pin SDA para I2C
    static constexpr int SCL = 22;      // Pin SCL para I2C
    static constexpr uint8_t ADDR = 0x27; // Dirección I2C típica para LCD
};

// Configuración de muestreo
constexpr unsigned long SAMPLE_RATE_HZ = 250;
constexpr unsigned long SAMPLE_INTERVAL_US = 1000000UL / SAMPLE_RATE_HZ;

// Configuración de filtrado
constexpr float DC_ALPHA = 0.995f;
constexpr int MA_WINDOW = 5;
constexpr int THRESHOLD = 40;

// Estructura para muestras ECG
struct ECGSample {
    int raw;           // Valor crudo del ADC
    float filtered;    // Valor filtrado
    unsigned long timestamp; // Marca de tiempo en millis()
};

// Estructura para datos de latido
struct BeatInfo {
    float bpm;
    unsigned long lastBeatTime;
    bool isInBeat;
};

#endif // ECG_TYPES_H