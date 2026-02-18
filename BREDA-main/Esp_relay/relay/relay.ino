#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define ERROR_PIN 13

uint8_t receptorMAC[] = {0x94, 0xE6, 0x86, 0x44, 0x9E, 0xC4};

typedef struct {
  uint8_t sync1;
  uint8_t sync2;
  uint8_t data[28];
} PacketData;

PacketData paquete;

void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    digitalWrite(ERROR_PIN, LOW);
  } else {
    digitalWrite(ERROR_PIN, HIGH);
  }
}

void setup() {
  pinMode(ERROR_PIN, OUTPUT);
  digitalWrite(ERROR_PIN, HIGH);
  
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    while(1);
  }
  
  esp_now_register_send_cb(onDataSent);
  
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, receptorMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error añadiendo peer");
    while(1);
  }
  
  Serial.println("ESP-NOW LR TX iniciado");
}

void loop() {
  if (Serial.available() >= 2) {
    uint8_t byte1 = Serial.read();
    
    if (byte1 != 0xFE) {
      return;
    }
    
    uint8_t byte2 = Serial.read();
    
    if (byte2 != 0xFB) {
      digitalWrite(ERROR_PIN, HIGH);
      delay(1);
      digitalWrite(ERROR_PIN, LOW);
      return;
    }
    
    while (Serial.available() < 28) {}
    
    Serial.readBytes(paquete.data, 28);
    
    bool pattern_found = false;
    for (int i = 0; i < 27; i++) {
      if (paquete.data[i] == 0xFE && paquete.data[i+1] == 0xFB) {
        pattern_found = true;
        break;
      }
    }
    
    if (pattern_found) {
      digitalWrite(ERROR_PIN, HIGH);
      delay(1);
      digitalWrite(ERROR_PIN, LOW);
      return;
    }
    
    paquete.sync1 = 0xFE;
    paquete.sync2 = 0xFB;
    
    digitalWrite(ERROR_PIN, HIGH);
    esp_now_send(receptorMAC, (uint8_t *) &paquete, sizeof(paquete));
  }
}