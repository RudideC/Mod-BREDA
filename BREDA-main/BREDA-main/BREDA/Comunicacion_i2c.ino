//-------------------------------------------------
//                I2C Communication
//-------------------------------------------------

uint8_t order_checking(uint8_t buf[], uint8_t length)
{
  // Size detection
  if (length < 7) 
    return -1;

  // ID detection
  if ((buf[0] != serialID[0]) || (buf[1] != serialID[1]) || (buf[2] != BREDA_ID))
    return -1;

  // Checksum 
  uint8_t checkSum = 0;
  for (int i = 4; i < length; ++i) 
    checkSum += buf[i];

  Serial.println(checkSum);
  if (checkSum != buf[3])
    return -1;

  return buf[6];    
}

void receiveEvent(int a) // Función ejecutada para activar la SD o el Relé
{  
  uint8_t buf[255] = {0}, size = 0;
  while (Wire1.available())
    buf[size++] = Wire1.read();

  // Orden detection
  order = order_checking(buf, size);

  if (order == 255) 
    order = -2;
    
  miniPack[6] = order + 100;
  obey_order(order);
}

///// Empaquetado datos termopares
union byteConverter
{
	float floatP;
	uint8_t byteP[4];
};

void requestEvent() // Función ejecutada cuando me solicita datos el Master
{ 
  pack_change();
  Wire1.write(miniPack, miniPackSize);
}

void pack_header()
{
  miniPack[0] = serialID[0];
  miniPack[1] = serialID[1];
  miniPack[2] = HERMES_ID;
  miniPack[4] = 78; 
  miniPack[5] = 1; 
}

void pack_change()
{
  // Error byte
  miniPack[7] = errorByte;

  noInterrupts();

  // Time
  float_to_byte(sec, 8);

  //Temp data
  for (int i = 0; i < 14; i++) {
    //if (abs(tempTP[i]) < 2000)
      float_to_byte(tempTP_raw[i], 12+4*i);
  }
  for (int i = 0; i < 4; i++) {
    if (abs(tempADC[i]) < 2000)
      float_to_byte(tempADC[i], 68+4*i);
  }
  interrupts();

  // CheckSum
  uint8_t check = 0;
  for (int i = 4; i < miniPackSize; ++i) check += miniPack[i];
  miniPack[3] = check;
}

void float_to_byte(float var, int pos)
{
  byteConverter conv;
  conv.floatP = var;
  for (int i = 0; i < 4; ++i) miniPack[pos + i] = conv.byteP[i]; 
}
 
//-------------------------------------------------
//              Order Interpretation
//-------------------------------------------------

void obey_order(uint8_t order)
{
  switch (order) 
  {
    // Conexion established
    case arduinoConnected:  
      transducer_enabled = true;
    break;

    // Transducer activated
    case enableTransducer:  
      transducer_enabled = true;
    break;

    // Transducer deactivated
    case disableTransducer: 
      transducer_enabled = false;
    break;

    // Hydrostatic activated
    case enableHydroStatic:  
      hydrostatic_enabled = true;
      break;

    // Hydrostatic deactivated
    case disableHydroStatic: 
      hydrostatic_enabled = false;
      break; 

    // Reboot
    case reboot:  
      performance_finished();  
    break;

    // Tare
    case tare:  
      #if TRANSDUCER == 1
        transducer_set_offset();
      #endif       
    break;
    
    // Start performing
    case startPerforming:  
      performance_started();
    break;

    // Stop performing
    case stopPerforming:  
      performance_finished();
    break;

    // Start ignition
    case startIgnition:  
      #if RELAY == 1
        ignition_started = true;
        //ignition_started = !ignition_started;
        power_relay(ignition_started);
        last_ignition = millis();
      #endif
    break;

    // Stop ignition (Centralita)
    case stopIgnition:  
      #if RELAY == 1
        power_relay(false);
        ignition_started = false;
      #endif
    break;
  }
}