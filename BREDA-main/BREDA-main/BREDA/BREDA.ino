#include "Config.h"

// Timer para envío a 40Hz (25ms de periodo)
unsigned long send_timer = 0;
const unsigned long send_interval = 25; // 25ms = 40Hz

void print_tp1()
{
  Serial.print("TP1: ");
  Serial.println(tempTP[0]);
}

void handle_order(uint8_t order)
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

void setup() 
{
  pinMode(A4, INPUT); 
  pinMode(PIN_LED_BUTTON, INPUT);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  Serial.begin(230400);     
  SPI.begin();              
  Serial4.begin(115200);    // UART4

  // SD Start
  #if SD_ON == 1
    if (!SDF.begin(SdioConfig(FIFO_SDIO))) 
    {
      errorByte |= B1;
      SD_ready = false;
      Serial.print("here");
    }
  #endif
  
  #if FLASH_ON == 1
    if (!flash_chip.begin()) 
    {
      errorByte |= B10;
      flash_ready = false;
    }
  #endif
  Serial.print("anda");
  
  // Transducer start
  #if TRANSDUCER == 1
    adc->adc0->setAveraging(8);
    adc->adc0->setResolution(12);
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);

    transducer_set_high_speed(false);
  #endif

  // ADCs
  ADC1.begin(ADC1_IRQ, ADC_clk_freq, vref);
  ADC2.begin(ADC2_IRQ, ADC_clk_freq, vref);
  ADC3.begin(ADC3_IRQ, ADC_clk_freq, vref);
  ADC4.begin(ADC4_IRQ, ADC_clk_freq, vref);

  ADC_config(ADC1, false);
  ADC_config(ADC2, false);
  ADC_config(ADC3, false);
  ADC_config(ADC4, true);

  performance_started(); 
}

void loop() 
{
  // Write it in the SD
  if (RB_data.bytesUsed() >= 512 && !file_data.isBusy())
  {
    RB_data.writeOut(512);
  }

  #if TRANSDUCER == 1 
    if (RB_pressure.bytesUsed() >= 512 && !file_pressure.isBusy())
    {
      RB_pressure.writeOut(512);
    }
  #endif

  // Transducer offset
  #if TRANSDUCER == 1 
    if ((transducer_offset_activated) && (millis() > (transducer_timeOffset + transducer_timerOffset)))
    {
      transducer_offset /= transducer_counter_offset;
      transducer_offset_activated = false;
    }
  #endif 

  // RECEPCIÓN de datos por UART4 (no bloqueante)
  if (Serial4.available() > 0)
  {
    uint8_t rcvd_byte = Serial4.read();
    handle_order(rcvd_byte);
  }

  // ENVÍO de datos a 40Hz por UART4
  if (millis() - send_timer >= send_interval)
  {
    send_timer = millis();
    send_sensor_data();
    print_tp1();
  }
  
  // Turns on and off the led each second
  if (millis() > LED_time + LED_timer)
  {
    LED_time = millis();
    LED_status = !LED_status;
    digitalWrite(PIN_LED_GREEN, LED_status);
  }
}
