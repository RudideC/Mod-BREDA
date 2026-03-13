//-------------------------------------------------
//                   LIBRARIES
//-------------------------------------------------
// ADCs
#include "MCP3564.h"

// I2C
#include <Wire.h> 

// SD
#include <SdFat.h>
#include <RingBuf.h>

// Flash
#include <LittleFS.h>

// Transducer
#include <ADC.h>
#include <ADC_util.h>
#include <DMAChannel.h>
#include <AnalogBufferDMA.h>
#include <FreeStack.h>

//-------------------------------------------------
//                    PINOUT
//-------------------------------------------------

#define PIN_LED_RED       38
#define PIN_LED_GREEN     39
#define PIN_LED_BUTTON    36
#define PIN_ALARM         41

#define PIN_CS_1          37
#define PIN_CS_2          31
#define PIN_CS_3          10
#define PIN_CS_4          33

#define PIN_IRQ_1         32
#define PIN_IRQ_2         30
#define PIN_IRQ_3         9
#define PIN_IRQ_4         34

#define PIN_MCLK_1        28
#define PIN_MCLK_2        29
#define PIN_MCLK_3        8
#define PIN_MCLK_4        35

#define PIN_TRANSDUCER    A4

#define BREDA_ADDRESS     0x35

#define UART_MAESTRO      Serial3  // Puerto UART para comunicación con maestro
#define UART_BAUD_RATE    115200   // Velocidad de comunicación

//-------------------------------------------------
//              SENSORS AND MODULES
//-------------------------------------------------

#define USE_ADC_0
#define TRANSDUCER                 1
#define SD_ON                      1
#define FLASH_ON                   0

//-------------------------------------------------
//                   VARIABLES
//-------------------------------------------------

// Serial3
const uint8_t UART_BUFFER_SIZE = 255;
uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
uint8_t uart_rx_index = 0;
bool uart_packet_ready = false;

// IDs
const uint8_t serialID[] = {0xFE, 0xFB};
const uint8_t BREDA_ID = 0xFC;
const uint8_t HERMES_ID = 0xFA;

// Times
uint32_t serial_time = 0;
uint32_t transducer_time = 0;
uint32_t transducer_timeOffset = 0;
uint32_t LED_time = 0;
uint32_t reset_time_ms = 0;
uint32_t reset_time_us = 0;
float sec = 0;

// Timers
uint32_t serial_timer = 1000;
uint32_t transducer_timer = 200;
uint32_t transducer_timerOffset = 0;
uint32_t LED_timer = 500;

// Performance
bool performance_status = false;
bool hydrostatic_enabled = false;
uint8_t order = 0;
uint8_t last_order = 0;

// LEDs
bool LED_status = true;

// Error byte
uint8_t errorByte = 0;

// Polinomios de los termopares tipo K
const int polisize = 4; // indica el tamaño del los arrays del coeficientes de k

// Range -270 °C to 0 °C
const float coefs_k_01[polisize]={-2.90037,
                                  2.44018E-3 ,
                                  1.14580E-9 ,
                                  -7.56273E-14};
// Range 0 °C to 1372 °C                   
const float coefs_k_02[polisize]= {0.000000e+00,
                                  2.5132785E-2,
                                  -6.0883423E-8,
                                  5.5358209E-13};
                                  //9.3720918E-18};

// Variables ADCs
const uint32_t ADC_clk_freq = 18750000;
const float internalTemp_const = 0.00040096; // constante de la temperatura interna de los ADCs
const float vref = 3.3; // voltaje de referencia
const uint32_t int24max = 16777216; // 2**24 
const float TP_const = ((vref / int24max) * 1000000) / 4; // constante para escalar los datos crudos del ADC a voltajes de los termopares

volatile float tempTP[14] = {0}; // array que almacena las temperaturas de los termopares
volatile int32_t tempTP_raw[14] = {0}; // array que almacena las temperaturas de los termopares
volatile float tempADC[4] = {0}; // array que almacena las temperaturas de los ADC
volatile int32_t thrust = 0; // almacena el valor del empuje
volatile int32_t prev_thrust = 1; // almacena el valor del empuje anterior

// File 
String general_file_header = "Time, Thrust, Pressure, ADC1_Temp, ADC2_Temp, ADC3_Temp, ADC4_Temp, TP_1, TP_2, TP_3, TP_4, TP_5, TP_6, TP_7, TP_8, TP_9";
String pressure_file_header = "Time, Chamber_Pressure, Raw_Transducer";
uint8_t file_number = 0;

// SD
const uint32_t RB_size = 20 * 512;
const uint32_t file_size = 100 * 1024;

bool file_closed = true;
bool SD_ready = true;
String file_data_name = "_Datos.txt";
String file_pressure_name = "_Presion.txt";

// Flash
bool flash_ready = true;

// Transducer variables
const uint16_t transducer_max_pressure = 250;
const uint16_t transducer_min_value = 496;
const uint16_t transducer_max_value = 2482;
const float transducer_const = float(transducer_max_pressure) / (transducer_max_value - transducer_min_value);

volatile uint32_t transducer_counter = 0;
volatile uint32_t transducer_counter_offset = 0;
volatile uint32_t transducer_freq = 0;

volatile int16_t transducer_raw = 0;
volatile int16_t pressure = 0;
volatile float transducer_avg = 0;
volatile float transducer_pressure = 0;

volatile int32_t transducer_offset = 0;

bool transducer_enabled = true;
bool transducer_offset_activated = false;

// HERMES orders
enum radio_orders 
{
  arduinoConnected = 1,
  enableTransducer,
  disableTransducer,
  enableHydroStatic,
  disableHydroStatic,
  reboot,
  rarete,
  startPerforming,
  stopPerforming,
  startIgnition = 11,
  stopIgnition,
  tare,
  errorSD = 20,
  errorSDFile,
};

// Byte converter
union byteToInt
{
  uint8_t bytes[4] = {0};
  int32_t integer;
};

// Pack deliver
const uint16_t miniPackSize = 84;
uint8_t miniPack[miniPackSize] = {0};

//-------------------------------------------------
//                OBJECT DECLARATION
//-------------------------------------------------¡

// Declaramos ADCs
MCP3564 ADC1(&SPI, PIN_CS_1, PIN_IRQ_1, PIN_MCLK_1);
MCP3564 ADC2(&SPI, PIN_CS_2, PIN_IRQ_2, PIN_MCLK_2);
MCP3564 ADC3(&SPI, PIN_CS_3, PIN_IRQ_3, PIN_MCLK_3);
MCP3564 ADC4(&SPI, PIN_CS_4, PIN_IRQ_4); // No declaramos el MCLK porque la salida de la teensy no es PWM (mirar librería para más info)

// Flash
#if FLASH_ON == 1
  LittleFS_QSPIFlash flash_chip;
  File flash_file;
#endif

// SD
#if SD_ON == 1
  SdFs SDF;
  FsFile file_data;
  
  // RingBuf for File type FsFile.
  RingBuf<FsFile, RB_size> RB_data;

  #if TRANSDUCER == 1
    FsFile file_pressure;
    RingBuf<FsFile, RB_size> RB_pressure;
  #endif
#endif

// Transducer
#if TRANSDUCER == 1
  ADC *adc = new ADC();
#endif
