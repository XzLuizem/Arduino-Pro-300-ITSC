/*
  UNO specific ECG reader for AD8232 (cloned modules)
  - Uses analogRead(A0)
  - Software sampling ~250 Hz (micros based)
  - Simple DC removal (IIR), moving average smoothing and R-peak detection
  - Prints raw/hp/lp and BEAT,BPM lines for debugging
*/

#include <Arduino.h>

const int ECG_PIN = A0;
const unsigned long SAMPLE_INTERVAL_US = 4000UL; // ~250 Hz
const int MA_WINDOW = 5;
int maBuf[MA_WINDOW];
int maIndex = 0;
long maSum = 0;

unsigned long lastMicros = 0;
float dcEstimate = 0.0;
const float DC_ALPHA = 0.995;

unsigned long lastBeatMillis = 0;
float bpm = 0;
int threshold = 20; // ajuste inicial en unidades ADC (0-1023)
bool inBeat = false;

void setup()
{
  Serial.begin(115200);
  pinMode(ECG_PIN, INPUT);
  for (int i = 0; i < MA_WINDOW; i++)
    maBuf[i] = 0;
  lastMicros = micros();
  dcEstimate = analogRead(ECG_PIN);
  Serial.println("UNO ECG started. Adjust 'threshold' if needed.");
}

void loop()
{
  unsigned long now = micros();
  if (now - lastMicros >= SAMPLE_INTERVAL_US)
  {
    lastMicros += SAMPLE_INTERVAL_US;
    int raw = analogRead(ECG_PIN);

    // DC removal (IIR)
    dcEstimate = DC_ALPHA * dcEstimate + (1.0 - DC_ALPHA) * raw;
    float hp = raw - dcEstimate;

    // moving average
    maSum -= maBuf[maIndex];
    maBuf[maIndex] = (int)hp;
    maSum += maBuf[maIndex];
    maIndex = (maIndex + 1) % MA_WINDOW;
    float lp = (float)maSum / MA_WINDOW;

    unsigned long nowMs = millis();
    if (lp > threshold && (nowMs - lastBeatMillis) > 250)
    {
      if (!inBeat)
      {
        inBeat = true;
        if (lastBeatMillis != 0)
        {
          unsigned long beatInterval = nowMs - lastBeatMillis;
          bpm = 60000.0 / beatInterval;
        }
        lastBeatMillis = nowMs;
        Serial.print("BEAT, BPM=");
        Serial.println((int)bpm);
      }
    }
    else if (lp < (threshold / 2))
    {
      inBeat = false;
    }

    // debug output
    Serial.print("raw:");
    Serial.print(raw);
    Serial.print(" hp:");
    Serial.print((int)hp);
    Serial.print(" lp:");
    Serial.print((int)lp);
    Serial.print(" bpm:");
    Serial.println((int)bpm);
  }
}
