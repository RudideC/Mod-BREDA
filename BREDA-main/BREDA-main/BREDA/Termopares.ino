//-------------------------------------------------
//                 ADC conversion
//-------------------------------------------------

int32_t ADC_getRaw(byteToInt buf)
{
  uint8_t sus; 
  sus = buf.bytes[0];
  buf.bytes[0] = buf.bytes[3];
  buf.bytes[3] = sus;
  sus = buf.bytes[2];
  buf.bytes[2] = buf.bytes[1];
  buf.bytes[1] = sus;

  if (buf.bytes[3] % 2 == 1) // cuando los integers son negativos, todo 1s son el numero negativo mas grande (es decir, lo más cercano a 0)
    buf.bytes[3] = B11111111;
  else
    buf.bytes[3] = 0;

  return buf.integer;
}

//-------------------------------------------------
//             Thermocouple conversion
//-------------------------------------------------

float TP_polynom(int32_t raw) {
  float TP_temp = 0;

  for (int i=polisize-1; i>=0; --i)
  {
    TP_temp = TP_temp * raw + coefs_k_01[i]; 
  }
  return TP_temp; // temperatura del termopar
}

//-------------------------------------------------
//                ADC Configuration
//-------------------------------------------------

void ADC_config(MCP3564& adc, bool ID_ADC4) {
  uint8_t conf[1];

  // Configuro el registro 1 (Pag 92 de la data sheet)
  if (ID_ADC4)
    conf[0] = B00011000; // ADC4 tiene menos Oversampling porque no tiene tanta velocidad de reloj 
  else
    conf[0] = B00100000; 
  adc.writeReg(conf, MCP3564_ADR_CONFIG1);

  // Configuro el registro 2 (Pag 93 de la data sheet)
  conf[0] = B10011001; 
  adc.writeReg(conf, MCP3564_ADR_CONFIG2);

  // Configuro el registro 3 (Pag 94 de la data sheet)
  conf[0] = B11110000; 
  adc.writeReg(conf, MCP3564_ADR_CONFIG3);

  // Configuro el registro del interrupt (IRQ) (Pag 95 de la data sheet)
  conf[0] = B01110011; 
  adc.writeReg(conf, MCP3564_ADR_IRQ);

  // Configuro el registro del modo SCAN (Pag 97 de la data sheet)
  uint16_t channelMask = 0;
  if (ID_ADC4)
    channelMask = B00011011 << 8; // ADC4 no tiene el CH4-CH5 activado
  else
    channelMask = B00011111 << 8; 
  adc.setScanSettings(0, channelMask); // mira en la librería para ver como funciona

  // Configuro el registro 0 (Pag 91 de la data sheet)
  //if (ID_ADC4)
    conf[0] = B00100011; // ADC4 no tiene entrada PWM
  //else
  //  conf[0] = B00000011; 
  adc.writeReg(conf, MCP3564_ADR_CONFIG0);
}

//-------------------------------------------------
//                 ADC Interrupts
//-------------------------------------------------

void ADC1_IRQ() { // declara la funcion de lectura del IRQ
  
  byteToInt temp_buffer; //Tambien podría poner {0} para inicializar todo en 0

  ADC1.readReg(temp_buffer.bytes, MCP3564_ADR_ADCDATA, 4); // leo el registro que almacena los datos 
  uint8_t chID = (temp_buffer.bytes[0] >> 4) & (B0111); // los bits del chanel ID son los 4 de la izquierda
  int32_t temp = ADC_getRaw(temp_buffer);

  if (chID == 4) // temperatura interna
    tempADC[0] = internalTemp_const * vref * temp - 269.13; // guarda la temperatura del ADC1 en la posición primera del array
  else
  {
    tempTP_raw[chID] = temp;
    tempTP[chID] = TP_polynom(temp) + tempADC[0];
  }
  
  if (performance_status) 
  {
    SD_file_data_update();  // SD
    flash_file_update(); // Flash
  }    
} 

void ADC2_IRQ() { // declara la funcion de lectura del IRQ
  
  byteToInt temp_buffer; //Tambien podría poner {0} para inicializar todo en 0

  ADC2.readReg(temp_buffer.bytes, MCP3564_ADR_ADCDATA, 4); // leo el registro que almacena los datos 
  uint8_t chID = (temp_buffer.bytes[0] >> 4) & (B0111); // los bits del chanel ID son los 4 de la izquierda
  int32_t temp = ADC_getRaw(temp_buffer);

  if (chID == 4) // temperatura interna
    tempADC[1] = internalTemp_const * vref * temp - 269.13; // guarda la temperatura del ADC1 en la posición primera del array
  else
  {
    tempTP_raw[chID + 4] = temp;
    tempTP[chID + 4] = TP_polynom(temp) + tempADC[0];
  }
} 

void ADC3_IRQ() { // declara la funcion de lectura del IRQ
  
  byteToInt temp_buffer; //Tambien podría poner {0} para inicializar todo en 0

  ADC3.readReg(temp_buffer.bytes, MCP3564_ADR_ADCDATA, 4); // leo el registro que almacena los datos 
  uint8_t chID = (temp_buffer.bytes[0] >> 4) & (B0111); // los bits del chanel ID son los 4 de la izquierda
  int32_t temp = ADC_getRaw(temp_buffer);

  if (chID == 4) // temperatura interna
    tempADC[2] = internalTemp_const * vref * temp - 269.13; // guarda la temperatura del ADC1 en la posición primera del array
  else
  {
    tempTP_raw[chID + 8] = temp;
    tempTP[chID + 8] = TP_polynom(temp) + tempADC[0];
  }
} 

void ADC4_IRQ() { // declara la funcion de lectura del IRQ
  
  byteToInt temp_buffer; //Tambien podría poner {0} para inicializar todo en 0

  ADC4.readReg(temp_buffer.bytes, MCP3564_ADR_ADCDATA, 4); // leo el registro que almacena los datos 
  uint8_t chID = (temp_buffer.bytes[0] >> 4) & (B0111); // los bits del chanel ID son los 4 de la izquierda
  int32_t temp = ADC_getRaw(temp_buffer);

  if (chID == 4) // temperatura interna
    tempADC[3] = internalTemp_const * vref * temp - 269.13; // guarda la temperatura del ADC1 en la posición primera del array
  else if (chID == 3) // celula de carga
    thrust = temp;
  else
  {
    tempTP_raw[chID + 12] = temp;
    tempTP[chID + 12] = TP_polynom(temp) + tempADC[0];
  }
} 