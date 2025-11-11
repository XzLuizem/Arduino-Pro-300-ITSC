#include <Arduino.h>
#include "BluetoothSerial.h"

// Create a BluetoothSerial object
BluetoothSerial SerialBT;

// Define the onboard LED pin for ESP32
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup()
{
  Serial.begin(115200); // Start serial communication with the PC
  SerialBT.begin("ESP32_LED_Control"); // Start Bluetooth with a device name
  Serial.println("Bluetooth device is ready to pair");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  if (SerialBT.available()) // If there is data available from Bluetooth
  {
    String command = SerialBT.readStringUntil('\n'); // Read the command string
    command.trim(); // Remove any whitespace

    Serial.print("Received: "); // Show in the serial monitor
    Serial.println(command);
    
    // Control the LED with the received commands
    if (command == "LED_ON")
    {
      digitalWrite(LED_BUILTIN, HIGH);
      SerialBT.println("LED ON");
    }
    else if (command == "LED_OFF")
    {
      digitalWrite(LED_BUILTIN, LOW);
      SerialBT.println("LED OFF");
    }
    else
    {
      SerialBT.print("Unknown command: ");
      SerialBT.println(command);
    }
  }
}