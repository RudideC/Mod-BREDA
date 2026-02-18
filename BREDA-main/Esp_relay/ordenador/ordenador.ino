#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define ERROR_PIN 13
#define BUFFER_SIZE 30

typedef struct {
  uint8_t sync1;
  uint8_t sync2;
  uint8_t data[28];
} PacketData;

uint8_t paquetes_buffer[BUFFER_SIZE][28];
uint8_t write_index = 0;
uint8_t read_index = 0;
uint8_t paquetes_disponibles = 0;

unsigned long ultimo_envio = 0;
const unsigned long intervalo_envio = 25;  // 40Hz

unsigned long paquetes_recibidos = 0;
unsigned long ultimo_reporte = 0;

void onDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
  if (len != sizeof(PacketData)) {
    Serial.print("ERROR: Tamaño incorrecto recibido: ");
    Serial.print(len);
    Serial.print(" bytes (esperaba ");
    Serial.print(sizeof(PacketData));
    Serial.println(" bytes)");
    
    digitalWrite(ERROR_PIN, HIGH);
    delay(1);
    digitalWrite(ERROR_PIN, LOW);
    return;
  }
  
  PacketData paquete;
  memcpy(&paquete, incomingData, sizeof(paquete));
  
  if (paquete.sync1 != 0xFE || paquete.sync2 != 0xFB) {
    Serial.print("ERROR: Sync incorrecto - recibido 0x");
    Serial.print(paquete.sync1, HEX);
    Serial.print(" 0x");
    Serial.println(paquete.sync2, HEX);
    
    digitalWrite(ERROR_PIN, HIGH);
    delay(1);
    digitalWrite(ERROR_PIN, LOW);
    return;
  }
  
  // Imprimir datos recibidos
  Serial.print("RX [");
  Serial.print(paquetes_recibidos++);
  Serial.print("] de MAC: ");
  for (int i = 0; i < 6; i++) {
    if (recv_info->src_addr[i] < 0x10) Serial.print("0");
    Serial.print(recv_info->src_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" | RSSI: ");
  Serial.print(recv_info->rx_ctrl->rssi);
  Serial.print(" dBm | Datos: ");
  
  for (int i = 0; i < 28; i++) {
    if (paquete.data[i] < 0x10) Serial.print("0");
    Serial.print(paquete.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Bufferear si hay espacio
  if (paquetes_disponibles < BUFFER_SIZE) {
    memcpy(paquetes_buffer[write_index], paquete.data, 28);
    write_index = (write_index + 1) % BUFFER_SIZE;
    paquetes_disponibles++;
  } else {
    Serial.println("ADVERTENCIA: Buffer lleno, descartando paquete");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(ERROR_PIN, OUTPUT);
  digitalWrite(ERROR_PIN, LOW);
  
  Serial.println("\n=== ESP-NOW LR Receptor ===");
  
  WiFi.mode(WIFI_STA);
  
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  
  Serial.print("MAC RX: ");
  Serial.println(WiFi.macAddress());
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: Fallo al inicializar ESP-NOW");
    while(1);
  }
  
  esp_now_register_recv_cb(onDataRecv);
  
  Serial.println("ESP-NOW LR RX iniciado - Esperando datos...\n");
}

void loop() {
  // Reporte de estadísticas cada 5 segundos
  if (millis() - ultimo_reporte >= 5000) {
    ultimo_reporte = millis();
    Serial.print("--- Stats: ");
    Serial.print(paquetes_recibidos);
    Serial.print(" paquetes | Buffer: ");
    Serial.print(paquetes_disponibles);
    Serial.print("/");
    Serial.print(BUFFER_SIZE);
    Serial.println(" ---");
  }
  
  if (paquetes_disponibles > 0 && (millis() - ultimo_envio >= intervalo_envio)) {
    ultimo_envio = millis();
    
    Serial.write(0x01);
    for (int j = 0; j < 28; j++) {
      Serial.write(paquetes_buffer[read_index][j]);
    }
    
    read_index = (read_index + 1) % BUFFER_SIZE;
    paquetes_disponibles--;
  }
}
