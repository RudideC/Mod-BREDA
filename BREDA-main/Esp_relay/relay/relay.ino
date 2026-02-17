#include <SPI.h>
#include <LoRa.h>

#define ERROR_PIN 13

static uint8_t buffer[210];  // 7 × (2 sync + 28 datos) = 210 bytes
static uint8_t paquetes_acumulados = 0;

void setup() {
  pinMode(ERROR_PIN, OUTPUT);
  digitalWrite(ERROR_PIN, LOW);

  Serial.begin(115200);  // UART hacia Teensy

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
  if (packetSize) {
    while (LoRa.available()) {
      uint8_t command = LoRa.read();
      Serial.write(command);  // Reenvía a Teensy
    }
  }

  if (Serial.available() >= 2) {
    uint8_t byte1 = Serial.read();
    
    if (byte1 != 0xFE) {
      return;
    }
    
    uint8_t byte2 = Serial.read();
    
    if (byte2 != 0xFB) {
      digitalWrite(ERROR_PIN, HIGH);
      digitalWrite(ERROR_PIN, LOW);
      return;
    }
    
    // Esperar 28 bytes
    while (Serial.available() < 28) {}
    
    uint8_t packet[28];
    Serial.readBytes(packet, 28);
    
    bool pattern_found = false;
    for (int i = 0; i < 27; i++) {
      if (packet[i] == 0xFE && packet[i+1] == 0xFB) {
        pattern_found = true;
        break;
      }
    }
    
    if (pattern_found) {
      digitalWrite(ERROR_PIN, HIGH);
      digitalWrite(ERROR_PIN, LOW);
      return;
    }
    
    uint16_t offset = paquetes_acumulados * 30;
    buffer[offset] = 0xFE;
    buffer[offset + 1] = 0xFB;
    memcpy(&buffer[offset + 2], packet, sizeof(packet));
    
    paquetes_acumulados++;
    // max es 8, 9 con 28 bytes
    if (paquetes_acumulados >= 7) {
      LoRa.beginPacket();
      LoRa.write(buffer, sizeof(buffer));
      digitalWrite(ERROR_PIN, HIGH);
      LoRa.endPacket();
      digitalWrite(ERROR_PIN, LOW);
      memset(buffer, 0, sizeof(buffer));
      paquetes_acumulados = 0;
    }
  }
}
