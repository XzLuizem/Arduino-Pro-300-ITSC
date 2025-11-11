#include <Arduino.h>
#include <Stepper.h>

// Definir el numero de pasos por revolucion del motor
const int stepsPerRevolution = 2048;

// OJO: La biblioteca Stepper.h espera los pines en un orden específico: IN1, IN3, IN2, IN4
// Si conectaste los pines del Arduino 8, 9, 10, 11 a IN1, IN2, IN3, IN4 del driver ULN2003,
// el orden correcto para la biblioteca es: 8, 10, 9, 11
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

// Variables para controlar la velocidad y direccion
// La velocidad máxima realista para este motor con esta librería es ~15 RPM
int speed = 10;      // Velocidad inicial en RPM (un valor realista)
bool direction = 0;  // Direccion: 0 para horario, 1 para antihorario

void setup() {
  // Configurar la velocidad del motor
  myStepper.setSpeed(speed);

  // NOTA: No necesitas llamar a pinMode(), la biblioteca Stepper.h
  // ya configura los pines como SALIDA (OUTPUT) automáticamente.

  // Imprimir mensaje de inicio en el monitor serial
  Serial.begin(9600);
  Serial.println("Laboratorio 9: Control de Motores Paso a Paso");
  Serial.println("Presione 'a' para aumentar la velocidad (Max 15 RPM)");
  Serial.println("Presione 'd' para disminuir la velocidad (Min 1 RPM)");
  Serial.println("Presione '1' para cambiar la direccion.");
}

void loop() {
  // Leer entrada del usuario el monitor serial
  if (Serial.available() > 0) {
    char command = Serial.read();

    switch (command) {
      case 'a':  // Aumentar la velocidad
        speed += 1; // Aumentamos de 1 en 1
        if (speed > 15) speed = 15;  // Limitar la velocidad máxima realista
        myStepper.setSpeed(speed);
        Serial.print("Velocidad aumentada a: ");
        Serial.println(speed);
        break;

      case 'd':  // Disminuir la velocidad
        speed -= 1; // Disminuimos de 1 en 1
        if (speed < 1) speed = 1;  // Limitar la velocidad mínima
        myStepper.setSpeed(speed);
        Serial.print("Velocidad disminuida a: ");
        Serial.println(speed);
        break;

      case '1':  // Cambiar la dirección (corregido de 'l' a '1')
        direction = !direction;
        Serial.print("Dirección cambiada a: ");
        Serial.println(direction ? "Antihorario" : "Horario");
        break;

      default:
        Serial.println("Comando no reconocido.");
        break;
    }
  }

  // Girar el motor un solo paso a la vez.
  // Esto permite que el loop() se ejecute muy rápido,
  // leyendo el puerto serial constantemente sin bloqueo.
  // La velocidad real la controla internamente myStepper.setSpeed().
  if (direction) { // 1 = Antihorario
    myStepper.step(-1); // Usamos un valor negativo para girar al revés
  } else {         // 0 = Horario
    myStepper.step(1);  // Usamos un valor positivo para girar
  }

  // delay(1000); // Breve pausa para comandos del motor.
}