//-------------------------------------------------
//              Funciones generales
//-------------------------------------------------
void error_warning() 
{
  for (int i=0; i<20; i++)
  {
    digitalWrite(PIN_LED_RED, false);
    delay(200);
    digitalWrite(PIN_LED_RED, true);
    delay(200);
  }
}

void power_relay(bool status)
{
  if (status) 
    tone(PIN_ALARM, 100, 5000);
} 

//-------------------------------------------------
//            Inicio y Final del Ensayo
//-------------------------------------------------

void send_sensor_data()
{
  // Buffer para construir el paquete
  uint8_t buffer[30];  // 2 bytes sincro + 28 bytes datos
  uint8_t idx = 0;
  
  // BYTES DE SINCRONIZACIÓN
  buffer[idx++] = 0xFE;
  buffer[idx++] = 0xFB;
  
  // Copiar datos con interrupciones deshabilitadas
  noInterrupts();
  
  // Timestamp en milisegundos (4 bytes - uint32_t)
  uint32_t timestamp = millis();
  memcpy(&buffer[idx], &timestamp, 4);
  idx += 4;
  
  // Thrust/Empuje (2 bytes - multiplicar × 100 para 2 decimales)
  int16_t thrust_int = (int16_t)(thrust * 100.0f);
  memcpy(&buffer[idx], &thrust_int, 2);
  idx += 2;
  
  // 10 Temperaturas de termopares (20 bytes - multiplicar × 100)
  for (int i = 0; i < 10; i++)
  {
    int16_t temp_int = (int16_t)(tempTP[i] * 100.0f);
    memcpy(&buffer[idx], &temp_int, 2);
    idx += 2;
  }
  
  // Transductor raw (2 bytes - dato crudo del ADC)
  memcpy(&buffer[idx], &transducer_raw, 2);
  idx += 2;
  
  interrupts();

  // Enviar todo el paquete de una vez
  Serial4.write(buffer, sizeof(buffer));
    
  // Debug opcional (comentar en producción)
  
  Serial.print("[SENT] Time: ");
  Serial.print(timestamp);
  Serial.print(" ms | Thrust: ");
  Serial.print(thrust);
  Serial.print(" | Pressure: ");
  Serial.println(transducer_avg);
}

//-------------------------------------------------
//        Función auxiliar para enviar floats
//-------------------------------------------------

void send_float(float value)
{
  union {
    float floatVal;
    uint8_t bytes[4];
  } converter;
  
  converter.floatVal = value;
  Serial3.write(converter.bytes, 4);
}

void performance_started()
{
  performance_status = true;

  SD_file_open();
  flash_file_open();

  transducer_set_high_speed(true);
}

void performance_finished()
{
  if (!file_closed) 
  {
    SD_file_close();
    flash_file_close();
    file_closed = true;
  }
      
  power_relay(false);
  transducer_set_high_speed(false);
  
  performance_status = false;

  reset_time_ms = millis();
  reset_time_us = reset_time_ms / 1000.;
}

void print_serial_TP()
{
  serial_time = millis();

  for (int i=0; i<4; ++i)
  {
    Serial.print("TemADC_");    
    Serial.print(i+1);
    Serial.print(": "); 
    Serial.print(tempADC[i]);
    Serial.print(" / ");

    for (int j=i*4; j<i*4+4; ++j)
    {
      if (j < 14)
      {
        Serial.print("TP_");    
        Serial.print(j+1);
        Serial.print(": "); 
        Serial.print(tempTP[j]);
        Serial.print(" , "); 
        Serial.print(tempTP_raw[j]);
        Serial.print(" / ");
      }
      else
      {
        break;
      }
    }
    Serial.println();
  }

  Serial.print("Célula: ");
  Serial.println(thrust);
  Serial.print("Transductor: ");
  Serial.println(transducer_avg / transducer_counter);
  Serial.println("---------------------------------------------------------");
}