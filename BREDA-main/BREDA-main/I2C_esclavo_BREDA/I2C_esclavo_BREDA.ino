// Comunicación i2c - Teensy 4.1 - BREDA (SLAVE)

#include <Wire.h> // Librería del i2c

// Define Slave I2C Address - Dirección de I2C del esclavo
#define SLAVE_ADDR 0x35   // Te dirige al periférico (sensor), no aún a la dirección de memoria/registro es por asi decirlo (Dirección del dispositivo esclavo en la red I2C)
 
union floatToByte         // esta unión vale para traducir por asi decirlo los floats a bytes y poder enviarlos comodamente por i2c
{
  float value;            // van a pillar este por default al declararlas
  uint8_t bytes[4]={0};
};

volatile floatToByte tempTP[14] = {10.8};  // guarda la temperatura de los termopares
volatile floatToByte tempTERM = {-1771};  // guarda la temperatura del termistor
volatile floatToByte pressure = {50.4};   // guarda la presión del transductor
volatile floatToByte thrust = {3000.54};  // guarda el dato de empuje

bool sd_ON = false;  // Variable booleana para la activación del guardado en la SD de datos
bool relay = false;  // activa o desactiva el relé de la ignición

void setup(){
  Wire1.begin(SLAVE_ADDR);  // Me quedo con la dirección I2C, 9, tengo como má´ximo 256 direcciones
  
  Wire1.onReceive(receiveEvent); // Registra la función a ejecutar al recibir datos desde el maestro

  Wire1.onRequest(requestEvent); // Registra la función a ejecutar cuando el maestro solicita datos

  Serial.begin(9600); // Inicializa la comunicación serial a 9600 baudios

  Serial.println(tempTERM.value,BIN);
  Serial.println(tempTERM.bytes[0],BIN);
  Serial.println(tempTERM.bytes[1],BIN);
  Serial.println(tempTERM.bytes[2],BIN);
  Serial.println(tempTERM.bytes[3],BIN);

  Serial.println("Fin del Setup");
}

void requestEvent() {

//  Wire1.write(98);
/*
  for (int i=0; i<14; ++i)
  {
    Serial.print("TP_");
    Serial.print(i+1);
    Serial.print(":   ");
    for (int j=0; j<4; ++j)
    {
      Wire1.write(tempTP[i].bytes[j]);  // Separamos y enviamos la información byte a byte cada termopar
      Serial.print(tempTP[i].bytes[j],BIN);
      Serial.print(" ");
    }
    Serial.println();
  }

*/
  Serial.print("TERM:  ");
  for (int j=0; j<4; ++j) // ojo que debería empezar en 0
  {
    Wire1.write(tempTERM.bytes[j]);  // Separamos y enviamos la información byte a byte
    Serial.print(tempTERM.bytes[j],BIN);
    Serial.print(" ");
  }
  Serial.println();


  Serial.print("PRESSURE:  ");
  for (int j=0; j<4; ++j)
  {
    Wire1.write(pressure.bytes[j]);
    Serial.print(pressure.bytes[j],BIN);
    Serial.print(" ");
  }
  Serial.println();


  Serial.print("THRUST:  ");
  for (int j=0; j<4; ++j)
  {
    Wire1.write(thrust.bytes[j]);
    Serial.print(thrust.bytes[j],BIN);
    Serial.print(" ");
  }
  Serial.println();
}


void receiveEvent(int size) // Función ejecutada para activar la SD o el Relé
{  
  Serial.print("Orden del Master:  ");
  bool sd_ON = Wire1.read();                // booleano que recibe del master para activar la SD
  bool relay = Wire1.read();                // booleano que recibe del master para activar el relé
  Serial.println("sd_ON y relay_ON");
}


void loop() 
{
 // delay(500);
}