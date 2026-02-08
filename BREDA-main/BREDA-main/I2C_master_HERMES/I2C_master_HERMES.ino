// Comunicación i2c - Teensy 4.1 - HERMES (MASTER)

#include <Wire.h> // Librería del i2c

union floatToByte  // esta unión vale para traducir por asi decirlo los floats a bytes y poder enviarlos comodamente por i2c
{
  float value;  // van a pillar este por default al declararlas
  uint8_t bytes[4]={0};
};

volatile floatToByte tempTP[14];  // guarda la temperatura de los termopares
volatile floatToByte tempTERM;  // guarda la temperatura del termistor
volatile floatToByte pressure;  // guarda la presión del transductor
volatile floatToByte thrust;  // guarda el dato de empuje

bool SD_on = true;
bool relay_on = true;

void setup() {

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  Wire.begin(); // Inicializo la comunicación I2C del master
  Serial.begin(9600);
  
  Serial.println("Fin del Setup");

  Wire.beginTransmission(0x35);
  Wire.write(SD_on);
  Serial.println("Orden de grabación de datos enviada");
  Wire.write(relay_on);
  Serial.println("Orden de ignición enviada");
  Wire.endTransmission();

  Serial.println("Inicio del ensayo");
}

void loop() {
  
  Wire.requestFrom(0x35, 12);
  //Serial.println(Wire.available()); 
  while (Wire.available())
  {
    //Serial.println(Wire.read());

    Serial.print("TERM:  ");
    for (int j=0; j<4; ++j) // ojo que debería empezar en 0
    {
      tempTERM.bytes[j] = Wire.read();  // Separamos y enviamos la información byte a byte
      Serial.print(tempTERM.bytes[j],BIN);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("TERM:    ");
    Serial.println(tempTERM.value);


    Serial.print("PRESSURE:  ");
    for (int j=0; j<4; ++j)
    {
      pressure.bytes[j] = Wire.read();  // Separamos y enviamos la información byte a byte
      Serial.print(pressure.bytes[j],BIN);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("PRESSURE:    ");
    Serial.println(pressure.value);

    Serial.print("THRUST:  ");
    for (int j=0; j<4; ++j)
    {
      thrust.bytes[j] = Wire.read();  // Separamos y enviamos la información byte a byte
      Serial.print(thrust.bytes[j],BIN);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("THRUST:    ");
    Serial.println(thrust.value);
    
  }
  Serial.println("----");

}
