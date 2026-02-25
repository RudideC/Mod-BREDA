#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_timer.h>  

#define ERROR_PIN    13
#define PIN_IGNICION 33
#define COMANDO_IGNICION 0x04

uint8_t ordenadorMAC[] = {0x94, 0xE6, 0x86, 0x44, 0x9E, 0xC4};

typedef struct {
  uint8_t sync1;
  uint8_t sync2;
  uint8_t data[28];
} PacketData;

typedef struct {
  uint8_t comando;
} PacketComando;

PacketData paquete;
volatile bool paquete_listo = false;   
volatile bool enviar_ahora = false;    

void IRAM_ATTR timer_callback(void* arg) {
  enviar_ahora = true;
}

void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    digitalWrite(ERROR_PIN, HIGH);
  }
}

void onDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
  if (len != sizeof(PacketComando)) return;

  PacketComando cmd;
  memcpy(&cmd, incomingData, sizeof(cmd));

  if (cmd.comando == COMANDO_IGNICION) {
    digitalWrite(PIN_IGNICION, HIGH);
  } else {
    Serial.write(cmd.comando);
  }
}

void setup() {
  pinMode(ERROR_PIN, OUTPUT);
  digitalWrite(ERROR_PIN, HIGH);

  pinMode(PIN_IGNICION, OUTPUT);
  digitalWrite(PIN_IGNICION, LOW);

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(88);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_LORA_250K);

  if (esp_now_init() != ESP_OK) while(1);

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, ordenadorMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) while(1);

  const esp_timer_create_args_t timer_args = {
    .callback = &timer_callback,
    .name = "tx_timer"
  };
  esp_timer_handle_t timer_handle;
  esp_timer_create(&timer_args, &timer_handle);
  esp_timer_start_periodic(timer_handle, 12500);  
}

void loop() {
  if (Serial.available() >= 2) {
    uint8_t byte1 = Serial.read();

    if (byte1 != 0xFE) return;

    uint8_t byte2 = Serial.read();

    if (byte2 != 0xFB) {
      digitalWrite(ERROR_PIN, HIGH);
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
      digitalWrite(ERROR_PIN, LOW);
      return;
    }

    paquete.sync1 = 0xFE;
    paquete.sync2 = 0xFB;
    paquete_listo = true;  
  }

  if (enviar_ahora) {
    enviar_ahora = false;

    if (paquete_listo) {
      esp_now_send(ordenadorMAC, (uint8_t *) &paquete, sizeof(paquete));
      paquete_listo = false;
    }
  }
}
