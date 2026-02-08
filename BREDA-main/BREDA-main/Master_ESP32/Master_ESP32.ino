// Include Arduino Wire library for I2C
#include <Wire.h>
 
#define SLAVE_ADDR 9  // Define Slave I2C Address
 
//----------------------------------------------------------------------

void setup() {
  Wire.begin(); // Inicia la comunicación I2C como maestro
  Serial.begin(9600); // Inicia la comunicación Serial (a 9600 baudios /  bits mamenos)
  Serial.println("Iniciando comunicación I2C como maestro");
}

void loop() {
  sendRequestToSlave(); // Función para enviar una solicitud de datos al esclavo
  receiveResponseFromSlave(); // Función para recibir datos del esclavo
  delay(1000); // Espera un segundo antes de repetir el proceso
}

//-----------------------------------------------------------------------

// Función para solicitar datos del esclavo
void sendRequestToSlave() {
  Wire.beginTransmission(SLAVE_ADDR); // Inicia la transmisión con el esclavo en la dirección determinada de i2c
  Wire.write("Dame datos esclavo"); // Envía una petición al esclavo
  Wire.endTransmission(); // Termina la transmisión
  Serial.println("Petición enviada al esclavo (por parte del master)");
}
 
//--------------------------------------------------------------------------

// Función para recibir una respuesta del esclavo
void receiveResponseFromSlave() {
  Wire.requestFrom(SLAVE_ADDR, 32); // Solicita (hasta) 32 bytes del esclavo
  Serial.print("Respuesta del esclavo (recibida por el master): ");
  while (Wire.available()) { // Mientras haya datos disponibles para leer
    char c = Wire.read(); // Lee un byte
    Serial.print(c); // Imprime el byte recibido
  }
  Serial.println(); // Finaliza la línea para una mejor visualización
}