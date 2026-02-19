#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_timer.h>

#define ERROR_PIN 13
#define BUFFER_SIZE 30

uint8_t relayMAC[] = {0xA4, 0xE5, 0x7C, 0xF6, 0xC2, 0x40};

typedef struct {
  uint8_t sync1;
  uint8_t sync2;
  uint8_t data[28];
} PacketData;

typedef struct {
  uint8_t comando;
} PacketComando;

uint8_t paquetes_buffer[BUFFER_SIZE][28];
volatile uint8_t write_index = 0;
uint8_t read_index = 0;
volatile uint8_t paquetes_disponibles = 0;

volatile bool enviar_ahora = false;

void IRAM_ATTR timer_callback(void* arg) {
  enviar_ahora = true;
}

void onDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
  if (len != sizeof(PacketData)) {
    digitalWrite(ERROR_PIN, HIGH);
    digitalWrite(ERROR_PIN, LOW);
    return;
  }

  PacketData paquete;
  memcpy(&paquete, incomingData, sizeof(paquete));

  if (paquete.sync1 != 0xFE || paquete.sync2 != 0xFB) {
    digitalWrite(ERROR_PIN, HIGH);
    digitalWrite(ERROR_PIN, LOW);
    return;
  }

  if (paquetes_disponibles < BUFFER_SIZE) {
    memcpy(paquetes_buffer[write_index], paquete.data, 28);
    write_index = (write_index + 1) % BUFFER_SIZE;
    paquetes_disponibles++;
  }
}

void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    digitalWrite(ERROR_PIN, HIGH);
    digitalWrite(ERROR_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ERROR_PIN, OUTPUT);
  digitalWrite(ERROR_PIN, LOW);

  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(88);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_LORA_250K);

  Serial.print("MAC RX: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) while(1);

  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, relayMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) while(1);

  // Timer a 50Hz = 20000µs
  const esp_timer_create_args_t timer_args = {
    .callback = &timer_callback,
    .name = "serial_out_timer"
  };
  esp_timer_handle_t timer_handle;
  esp_timer_create(&timer_args, &timer_handle);
  esp_timer_start_periodic(timer_handle, 20000);
}

void loop() {
  // Enviar datos al ordenador a 50Hz
  if (enviar_ahora) {
    enviar_ahora = false;

    if (paquetes_disponibles > 0) {
      Serial.write(0x01);
      for (int j = 0; j < 28; j++) {
        Serial.write(paquetes_buffer[read_index][j]);
      }
      read_index = (read_index + 1) % BUFFER_SIZE;
      paquetes_disponibles--;
    }
  }

  if (Serial.available()) {
    uint8_t comando = Serial.read();

    PacketComando cmd;
    cmd.comando = comando;
    esp_now_send(relayMAC, (uint8_t *) &cmd, sizeof(cmd));
  }
}
