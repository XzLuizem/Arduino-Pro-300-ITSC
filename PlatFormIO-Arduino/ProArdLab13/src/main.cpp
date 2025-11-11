#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_system.h>
#include "private_config.h"  // Configuración del dispositivo

// Credenciales Wi-Fi desde private_config.h
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

// Credenciales para la interfaz web desde private_config.h
const char* www_username = WWW_USERNAME;
const char* www_password = WWW_PASSWORD;

// Variables para tracking
unsigned long startTime = 0;
bool ledState = false;

const int ledPin = 13; // Pin digital para el LED

WebServer server(80);

// Reconexión Wi-Fi
bool serverStarted = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // intentos cada 5s

// Simple página web para controlar el LED desde el navegador
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 LED Control</title>
    <style>
      body { font-family: Arial, sans-serif; margin: 20px; text-align: center; }
      button { 
        padding: 10px 20px; 
        margin: 10px;
        font-size: 16px;
        cursor: pointer;
        background-color: #4CAF50;
        color: white;
        border: none;
        border-radius: 4px;
      }
      button:hover { background-color: #45a049; }
      button.off { background-color: #f44336; }
      button.off:hover { background-color: #da190b; }
      #status { margin: 20px; padding: 10px; }
      .info { 
        background: #f8f9fa;
        padding: 15px;
        margin: 20px auto;
        max-width: 400px;
        border-radius: 8px;
      }
    </style>
  </head>
  <body>
    <h1>ESP32 LED Control</h1>
    <div class="info">
      <p>Estado actual: <span id="currentState">Consultando...</span></p>
      <p>Tiempo activo: <span id="uptime">Consultando...</span></p>
    </div>
    <button onclick="controlLed('on')">Encender</button>
    <button onclick="controlLed('off')" class="off">Apagar</button>
    <div id="status"></div>
    <script>
      let authHeader = 'Basic ' + btoa('admin:esp32led');
      
      function controlLed(state) {
        const status = document.getElementById('status');
        status.textContent = 'Enviando comando...';
        
        fetch(`/led?state=${state}`, {
          headers: {
            'Authorization': authHeader
          }
        })
          .then(response => {
            if (!response.ok) throw new Error('Error de autenticación');
            return response.text();
          })
          .then(text => {
            status.textContent = text;
            updateStatus();
          })
          .catch(error => {
            status.textContent = `Error: ${error.message}`;
          });
      }

      function updateStatus() {
        fetch('/status', {
          headers: {
            'Authorization': authHeader
          }
        })
          .then(r => r.json())
          .then(data => {
            document.getElementById('currentState').textContent = data.led ? 'LED ENCENDIDO' : 'LED APAGADO';
            document.getElementById('uptime').textContent = data.uptime;
          })
          .catch(console.error);
      }

      // Actualizar estado cada 2 segundos
      updateStatus();
      setInterval(updateStatus, 2000);
    </script>
  </body>
</html>
)rawliteral";

// Función auxiliar para formatear el tiempo de actividad
String getUptimeString() {
  unsigned long currentTime = millis();
  unsigned long seconds = (currentTime - startTime) / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  char uptimeStr[64];
  snprintf(uptimeStr, sizeof(uptimeStr), "%lud %02lu:%02lu:%02lu", 
           days, hours % 24, minutes % 60, seconds % 60);
  return String(uptimeStr);
}

bool checkAuth() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    Serial.println("Intento de acceso no autorizado");
    return false;
  }
  return true;
}

void handleRoot() {
  if (!checkAuth()) return;
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.send_P(200, "text/html", INDEX_HTML);
  
  Serial.println("Página principal accedida");
}

void handleStatus() {
  if (!checkAuth()) return;
  
  if (WiFi.status() != WL_CONNECTED) {
    server.send(503, "application/json", "{\"error\":\"No WiFi connection\"}");
    return;
  }

  String json = "{\"led\":" + String(ledState ? "true" : "false") + 
                ",\"uptime\":\"" + getUptimeString() + "\"}";
                
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.send(200, "application/json", json);
}

void handleLed() {
  if (!checkAuth()) return;
  
  // Ejecutar sólo si hay conexión Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.send(503, "text/plain", "No WiFi connection");
    return;
  }
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  
  String state = server.arg("state");
  state.toLowerCase();
  if (state == "on") {
    digitalWrite(ledPin, HIGH);
    ledState = true;
    server.send(200, "text/plain", "LED encendido");
    Serial.println("LED encendido via web");
  } else if (state == "off") {
    digitalWrite(ledPin, LOW);
    ledState = false;
    server.send(200, "text/plain", "LED apagado");
    Serial.println("LED apagado via web");
  } else {
    server.send(400, "text/plain", "Parámetro 'state' inválido (on/off)");
  }
}

void setup()
{
  Serial.begin(115200);
  // pequeña pausa para permitir que el monitor serie se conecte (no bloqueante en ESP32)
  delay(10);

  startTime = millis();
  pinMode(ledPin, OUTPUT); // Configurar el pin del LED como salida
  digitalWrite(ledPin, LOW); // Iniciar apagado
  ledState = false;

  // Conectar a la red Wi-Fi con timeout para no bloquear indefinidamente
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  unsigned long start = millis();
  const unsigned long timeout = 10000; // 10 segundos
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout)
  {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    Serial.print("Conectado a WiFi, IP: ");
    Serial.println(WiFi.localIP());
    // Iniciar servidor HTTP para recibir comandos desde la web
    server.on("/", handleRoot);
    server.on("/led", handleLed);
    server.on("/status", handleStatus);
    if (!serverStarted) {
      server.begin();
      serverStarted = true;
      Serial.println("HTTP server iniciado en puerto 80");
    }
  }
  else
  {
    Serial.println();
    Serial.println("WiFi no conectado (timeout)");
  }
}

void loop()
{
  // Manejo del servidor HTTP cuando estemos conectados
  if (WiFi.status() == WL_CONNECTED) {
    // Asegurar que el servidor esté iniciado una vez
    if (!serverStarted) {
      server.begin();
      serverStarted = true;
      Serial.println("HTTP server iniciado en puerto 80");
    }
    server.handleClient();
  } else {
    // Si perdemos la conexión, marcamos el servidor como no iniciado para reiniciarlo al reconectar
    if (serverStarted) {
      serverStarted = false;
      Serial.println("WiFi desconectado: el servidor HTTP quedará inactivo hasta reconexión");
    }
    // Intento de reconexión no bloqueante
    unsigned long now = millis();
    if (now - lastReconnectAttempt >= reconnectInterval) {
      lastReconnectAttempt = now;
      Serial.println("Intentando reconectar a WiFi...");
      WiFi.begin(ssid, password);
    }
  }

  // Procesar comandos desde Serial pero ejecutar acciones sólo si hay Wi-Fi
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n'); // Leer el comando hasta nueva línea
    command.trim();
    if (WiFi.status() == WL_CONNECTED) {
      if (command.equalsIgnoreCase("LED_ON")) {
        digitalWrite(ledPin, HIGH);
        Serial.println("LED encendido");
      }
      else if (command.equalsIgnoreCase("LED_OFF")) {
        digitalWrite(ledPin, LOW);
        Serial.println("LED apagado");
      }
      else if (command.length() > 0) {
        Serial.print("Comando desconocido: ");
        Serial.println(command);
      }
    } else {
      Serial.println("No hay conexión WiFi: comando ignorado");
    }
  }

  // breve delay no bloqueante; para temporizaciones más complejas usar millis()
  delay(100);
}