# ESP32 LED Control con Web Server

Control remoto de LED a través de servidor web con autenticación y acceso remoto vía ngrok.

## Configuración

1. Copia el archivo de ejemplo de configuración:
   ```bash
   cp include/private_config.h.example include/private_config.h
   ```

2. Edita `include/private_config.h` con tus credenciales:
   ```cpp
   // Credenciales Wi-Fi
   #define WIFI_SSID "tu_ssid_wifi"
   #define WIFI_PASSWORD "tu_password_wifi"

   // Credenciales para la interfaz web
   #define WWW_USERNAME "admin"
   #define WWW_PASSWORD "tu_password_web"
   ```

## Características

- Control de LED via web con autenticación
- Acceso remoto vía ngrok
- Reconexión Wi-Fi automática
- Página de estado con uptime
- Logging de acciones
- CORS habilitado para acceso remoto

## Compilación y Upload

1. Instala PlatformIO (CLI o VS Code extension)
2. Compila:
   ```bash
   platformio run
   ```
3. Sube al ESP32:
   ```bash
   platformio run --target upload
   ```
4. Monitor serie:
   ```bash
   platformio device monitor -b 115200
   ```

## Acceso Remoto con ngrok

1. Instala ngrok
2. Cuando el ESP32 muestre su IP (ej: 192.168.1.100), ejecuta:
   ```bash
   ngrok http IP_DEL_ESP32:80
   ```
3. Usa la URL proporcionada por ngrok

## Seguridad

- Las credenciales están en `include/private_config.h` (no incluido en git)
- Autenticación requerida para todas las rutas
- CORS configurado para acceso remoto seguro
- Logging de accesos en Serial

## Endpoints

- `/` - Interfaz web
- `/led?state=on|off` - Control del LED
- `/status` - Estado del LED y uptime (JSON)

## Notas de Desarrollo

- El archivo `private_config.h` está en `.gitignore` para evitar exponer credenciales
- Usa `private_config.h.example` como plantilla para configuración
- Los logs de acceso se muestran en el monitor serie