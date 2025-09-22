#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11
//#define DHTTYPE DHT22 // DHT 22 (Selecciona el tipo de sensor)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Error leyendo los sensores!"));
    return;
  }
  Serial.print(F("Humedad: "));
  Serial.print(h);
  Serial.print(F(" %\t"));
  Serial.print(F("Temperatura: "));
  Serial.print(t);
  Serial.println(F(" *C "));
  delay(2000);
}
