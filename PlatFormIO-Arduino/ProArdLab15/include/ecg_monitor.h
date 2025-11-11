/**
 * @file ecg_monitor.h
 * @brief Gestor del monitor ECG
 */

#ifndef ECG_MONITOR_H
#define ECG_MONITOR_H

#include "ecg_types.h"
#include <CircularBuffer.h>

class ECGMonitor {
public:
    ECGMonitor() : 
        dcEstimate(0),
        beatInfo{0, 0, false},
        leadsConnected(false) {
        for (int i = 0; i < MA_WINDOW; i++) {
            maBuffer[i] = 0;
        }
    }

    void begin() {
        // Configurar pines de Lead-Off Detection
        pinMode(AD8232Pins::LO_PLUS, INPUT);
        pinMode(AD8232Pins::LO_MINUS, INPUT);
        pinMode(AD8232Pins::OUTPUT_PIN, INPUT);
        
        dcEstimate = analogRead(AD8232Pins::OUTPUT_PIN);
    }

    bool checkLeadsConnected() {
        // Los pines LO+ y LO- son HIGH cuando los electrodos están desconectados
        bool connected = (digitalRead(AD8232Pins::LO_PLUS) == LOW &&
                        digitalRead(AD8232Pins::LO_MINUS) == LOW);
        
        if (connected != leadsConnected) {
            leadsConnected = connected;
            // Reset de variables si hay cambio en la conexión
            if (!connected) {
                beatInfo = {0, 0, false};
                dcEstimate = 0;
            }
        }
        return leadsConnected;
    }

    bool processSample() {
        // Verificar conexión de electrodos
        if (!checkLeadsConnected()) {
            return false;
        }

        int raw = analogRead(AD8232Pins::OUTPUT_PIN);
        bool beatDetected = false;

        // DC removal
        dcEstimate = DC_ALPHA * dcEstimate + (1.0f - DC_ALPHA) * raw;
        float hp = raw - dcEstimate;

        // Moving average
        maSum -= maBuffer[maIndex];
        maBuffer[maIndex] = (int)hp;
        maSum += maBuffer[maIndex];
        maIndex = (maIndex + 1) % MA_WINDOW;
        float lp = (float)maSum / MA_WINDOW;

        // Beat detection
        unsigned long now = millis();
        if (lp > THRESHOLD && (now - beatInfo.lastBeatTime) > 250) {
            if (!beatInfo.isInBeat) {
                beatInfo.isInBeat = true;
                if (beatInfo.lastBeatTime != 0) {
                    unsigned long beatInterval = now - beatInfo.lastBeatTime;
                    beatInfo.bpm = 60000.0f / beatInterval;
                    beatDetected = true;
                }
                beatInfo.lastBeatTime = now;
            }
        } 
        else if (lp < (THRESHOLD / 2)) {
            beatInfo.isInBeat = false;
        }

        // Store sample
        samples.push(ECGSample{raw, lp, now});
        return beatDetected;
    }

    const BeatInfo& getBeatInfo() const {
        return beatInfo;
    }

    int getLastRawValue() const {
        return samples.last().raw;
    }

    float getLastFilteredValue() const {
        return samples.last().filtered;
    }

private:
    CircularBuffer<ECGSample, 250> samples; // 1 segundo de historia
    float dcEstimate;
    int maBuffer[MA_WINDOW];
    int maIndex = 0;
    long maSum = 0;
    BeatInfo beatInfo;
    bool leadsConnected = false;
};

#endif // ECG_MONITOR_H