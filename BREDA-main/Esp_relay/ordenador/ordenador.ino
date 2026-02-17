#include <SPI.h>
#include <LoRa.h>

#define ERROR_PIN 13
#define BUFFER_SIZE 30  // Buffer para 30 paquetes porque sí

uint8_t paquetes_buffer[BUFFER_SIZE][28];
uint8_t write_index = 0;
uint8_t read_index = 0;
uint8_t paquetes_disponibles = 0;

unsigned long ultimo_envio = 0;
const unsigned long intervalo_envio = 25;  // 40Hz

void setup() {
  Serial.begin(115200);
  
  pinMode(ERROR_PIN, OUTPUT);
  digitalWrite(ERROR_PIN, LOW);
  
  while (!Serial);

  LoRa.setPins(4, 15, 2);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(500E3);
  LoRa.setPreambleLength(4);
  LoRa.setTxPower(14);

  if (!LoRa.begin(868E6)) {
    while (1);
  }
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize == 210) {  // 7 × 30 bytes
    for (int i = 0; i < 7; i++) {
      // Leer bytes de sincronización
      uint8_t sync1 = LoRa.read();
      uint8_t sync2 = LoRa.read();
      
      if (sync1 != 0xFE || sync2 != 0xFB) {
        digitalWrite(ERROR_PIN, HIGH);
        digitalWrite(ERROR_PIN, LOW);
        
        while (LoRa.available()) {
          LoRa.read();
        }
        return;
      }
      
      if (paquetes_disponibles < BUFFER_SIZE) {
        for (int j = 0; j < 28; j++) {
          paquetes_buffer[write_index][j] = LoRa.read();
        }
        
        write_index = (write_index + 1) % BUFFER_SIZE;
        paquetes_disponibles++;
      } else {
        for (int j = 0; j < 28; j++) {
          LoRa.read();
        }
      }
    }
  }

  if (paquetes_disponibles > 0 && (millis() - ultimo_envio >= intervalo_envio)) {
    ultimo_envio = millis();
    
    Serial.write(0x01);
    Serial.write(paquetes_buffer[read_index], 28);
    
    read_index = (read_index + 1) % BUFFER_SIZE;
    paquetes_disponibles--;
  

  if (Serial.available()) {
    uint8_t command = Serial.read();
    
    LoRa.beginPacket();
    LoRa.write(command);
    LoRa.endPacket();
    
    while (Serial.available()) {
      Serial.read();
    }
  }
}
