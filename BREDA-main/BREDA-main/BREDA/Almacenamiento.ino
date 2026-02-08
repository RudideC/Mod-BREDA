//-------------------------------------------------
//               File Configuration
//-------------------------------------------------

void file_select_name() 
{
  String newName = file_number + file_data_name;
  bool f = true, s = true;

  while (f || s)
  {
    // Checks if the file exist either in the SD or flash to change it
    newName = ++file_number + file_data_name;
    #if FLASH_ON == 1
      f = flash_chip.exists(newName.c_str());
    #else
      f = false;
    #endif  
    #if SD_ON == 1
      s = SDF.exists(newName.c_str()); 
    #else 
      s = false;
    #endif  
    
    Serial.print("intento numero ");
    Serial.print(file_number);
    Serial.print(" ");
    Serial.print(f);
    Serial.print(", ");
    Serial.println(s);
  }
}

//-------------------------------------------------
//              SD File Configuration
//-------------------------------------------------

void SD_file_open()
{
  #if SD_ON == 1
    if (!SD_ready)
      return;

    file_select_name();
    String newName = file_number + file_data_name;
    if (!file_data.open(newName.c_str(), O_RDWR | O_CREAT))
    {
      errorByte |= B100;
      error_warning();
      SD_ready = false;
      return;
    }

    #if TRANSDUCER == 1
      newName = file_number + file_pressure_name;
      if (!file_pressure.open(newName.c_str(), O_RDWR | O_CREAT))
      {
        errorByte |= B1000;
        error_warning();
        SD_ready = false;
        return;
      }
    #endif

    file_closed = false;

    // File header
    file_data.println(general_file_header);

    // Allocate space to avoid huge delays
    file_data.preAllocate(file_size); 

    // Starts the Ring Buffs
    RB_data.begin(&file_data);
    
    #if TRANSDUCER == 1
      file_pressure.println(pressure_file_header);
      file_pressure.preAllocate(file_size);
      RB_pressure.begin(&file_pressure);
    #endif
  #endif  
}

void SD_file_close()
{
  #if SD_ON == 1
    // Empty the buffer, truncate the file size and close it
    RB_data.sync();
    file_data.truncate();
    file_data.close();

    #if TRANSDUCER == 1
      RB_pressure.sync();
      file_pressure.truncate();
      file_pressure.close();
    #endif

    file_closed = true;
  #endif  
}

//-------------------------------------------------
//                 SD File Updates
//-------------------------------------------------

void SD_file_data_update()
{
  #if SD_ON == 1
    if (!SD_ready) 
      return;

    noInterrupts();
    // Time
    RB_data.print(millis() / 1000., 3);
        
    // Cell data
    RB_data.print(", ");
    RB_data.print(thrust, 5);

    // Transducer data (Average)
    RB_data.print(", ");
    RB_data.print(transducer_avg / transducer_counter, 5);

    transducer_avg = 0;
    transducer_counter = 0;

    for (int i = 0; i < 4; ++i)
    {
      RB_data.print(", ");
      RB_data.print(tempADC[i]);
    }

    for (int i = 0; i < 14; ++i)
    {
      RB_data.print(", ");
      RB_data.print(tempTP[i]);   
    }

    RB_data.println();
    interrupts();
  #endif
}

void SD_file_pressure_update()
{
  #if TRANSDUCER == 1
    if (!SD_ready) {return;}

    // Transducer time (in s)
    float transducer_time = micros() / 10000000. - reset_time_us;
    RB_pressure.print(transducer_time, 6);
    RB_pressure.print(", ");

    // Transducer measure already converted
    RB_pressure.print(transducer_pressure);
    RB_pressure.print(", ");

    // Raw transducer data    
    RB_pressure.println(transducer_raw);
  #endif   
}

//-------------------------------------------------
//              Flash file configuration
//-------------------------------------------------

void flash_file_open() 
{
  #if FLASH_ON == 1
    String newName = file_number + file_data_name;
    flash_file = flash_chip.open(newName.c_str(), FILE_WRITE);

    // File header
    flash_file.println(general_file_header);
  #endif
}

void flash_file_close()
{
  #if FLASH_ON == 1
    flash_file.close();
  #endif  
}

void flash_erase_all() 
{
  #if FLASH_ON == 1
    flash_chip.quickFormat();
  #endif  
}

//-------------------------------------------------
//              Flash file updates
//-------------------------------------------------

void flash_file_update()
{
  #if FLASH_ON == 1
    if (!flash_file)
      return;

    noInterrupts();

    // Time
    flash_file.print(millis() / 1000., 3);

    // TP
    for (uint8_t i = 0; i < 14; ++i)
    {
      flash_file.print(", ");
      flash_file.print(tempTP[i]);
    }

    // ADC Temps
    for (uint8_t i = 0; i < 4; ++i)
    {
      flash_file.print(", ");
      flash_file.print(tempADC[i]);
    }
    flash_file.println();
    interrupts();
  #endif
}