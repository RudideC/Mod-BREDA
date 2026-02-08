#include "Config.h"

void setup() 
{
  pinMode(A4, INPUT); 
  pinMode(PIN_LED_BUTTON, INPUT);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  Serial.begin(230400);     // inicializamos serial a 230400 baudios
  SPI.begin();              // inicializamos la comunicación por SPI
  Serial3.begin(115200);     // iniciamos la comunicacion serial por la que se enviaran datos

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
    adc->adc0->setResolution(12); // set bits of resolution
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // change the conversion speed
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED); // change the sampling speed

    transducer_set_high_speed(false);
  #endif

  // ADCs
  ADC1.begin(ADC1_IRQ, ADC_clk_freq, vref); // inicializamos el ADC 1 
  ADC2.begin(ADC2_IRQ, ADC_clk_freq, vref); // inicializamos el ADC 2
  ADC3.begin(ADC3_IRQ, ADC_clk_freq, vref); // inicializamos el ADC 3 
  ADC4.begin(ADC4_IRQ, ADC_clk_freq, vref); // inicializamos el ADC 4 

  ADC_config(ADC1, false);
  ADC_config(ADC2, false);
  ADC_config(ADC3, false);
  ADC_config(ADC4, true); // el ADC4 tiene cosas distintas (sabe cosas)

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

  // logica serial para enviar paquete de datos, no me odien, ya se que no va aqui pero el dry run es la semana que viene
  if (Serial3.available() > 0)
  {
    while(Serial3.available() > 0){
      Serial3.read();
    }
    send_sensor_data();
  }
  
  // Turns on and off the led each second
  if (millis() > LED_time + LED_timer)
  {
    LED_time = millis();
    LED_status = !LED_status;
    digitalWrite(PIN_LED_GREEN, LED_status);
  }
}