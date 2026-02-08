#include "MCP3x6x.h"
#include <SPI.h>

#define PIN_CS_1    37
#define PIN_CS_2    31
#define PIN_CS_3    10
#define PIN_CS_4    33

#define PIN_IRQ_1   32
#define PIN_IRQ_2   30
#define PIN_IRQ_3   9
#define PIN_IRQ_4   34

const float vref = 3.3;
uint8_t data[4] = {0, 0, 0, 0}; // definimos un array de 4bytes 

bool tempOn = true; // creamos etsa variable por si queremos medir la temp interna al ppio o el voltaje del canal 0

// Declaramos ADCs
auto spiWrapper = &SPI;
MCP3564 ADC1(spiWrapper, PIN_CS_1, PIN_IRQ_1);
void ADC1_int() { ADC1.IRQ_handler(); }  // declara la funcion de lectura del IRQ

void setup() 
{
  // estas líneas de a continuación sirven para inicializar los pines de los adc que no se van a usar, porque no vamos a poner 
  // MCP3564 ADC1(spiWrapper, PIN_CS_i, PIN_IRQ_i) ni adc.begin; así evitamos que entren en conflicto. Cuando si lo usemos para cada ADC
  // podremos borrar las dos lineas, tanto las de pinMode como digitalWrite

  //pinMode(PIN_CS_1, OUTPUT);  // para el spi
  pinMode(PIN_CS_2, OUTPUT);
  pinMode(PIN_CS_3, OUTPUT);
  pinMode(PIN_CS_4, OUTPUT);

  //digitalWrite(PIN_CS_1, true);  // inician en "high" para que no entre con conflicto con el resto, cuando lo llamos pasa a "low"
  digitalWrite(PIN_CS_2, true);
  digitalWrite(PIN_CS_3, true);
  digitalWrite(PIN_CS_4, true);

  SPI.begin();  //  inicializamos la comunicación por SPI
  Serial.begin(230400);  // inicializamos serial a 230400 baudios

  if (!ADC1.begin(ADC1_int, 0, vref)) // inicializamos el ADC 1 y comprobamos que lo ha hecho correctamente
  {
    Serial.println("fallo");
  }

  if (tempOn) tempConfig();
  else ch1Config();

  /*
  data[0] = B00110000;
  ADC1.writeReg(data, MCP3564_ADR_CONFIG3);
  ADC1.readReg(data, MCP3564_ADR_CONFIG3);
  Serial.print("Config3: ");
  Serial.println(data[0], BIN);
  */

  ADC1.readReg(data, MCP3564_ADR_IRQ, 4);  // leo el registro de MCP3564_ADR_IRQ de 4 bytes y lo cargo en el puntero data
  Serial.print("IRQ primero: ");
  Serial.println(data[0], BIN);
  delay(500);

  data[0] = B01100011;
  ADC1.writeReg(data, MCP3564_ADR_CONFIG0);
  ADC1.readReg(data, MCP3564_ADR_CONFIG0);
  Serial.print("Config0: ");
  Serial.println(data[0], BIN);
  delay(100);
  

  //ADC1.readReg(data, MCP3564_ADR_ADCDATA, 4);
  //for(int i = 0; i < 4; ++i) Serial.println(data[i], BIN);
}

void loop() 
{
  ADC1.readReg(data, MCP3564_ADR_IRQ, 4);
  //Serial.print("IRQ: ");
  //Serial.println(data[0], BIN);
  delay(500);
  if (data[0] == B00110011)
  {
    if (tempOn) tempProcessing();
    else ch1Processing();
  }
}

void tempConfig()
{
  data[0] = 0xDE;
  ADC1.writeReg(data, MCP3564_ADR_MUX);
  ADC1.readReg(data, MCP3564_ADR_MUX);
  Serial.print("Mux: ");
  Serial.println(data[0], BIN);
}

void ch1Config()
{
  data[0] = B00001100;
  ADC1.writeReg(data, MCP3564_ADR_MUX);
  ADC1.readReg(data, MCP3564_ADR_MUX);
  Serial.print("Mux: ");
  Serial.println(data[0], BIN);
}

void tempProcessing()
{
  //Serial.println("Funciona: ");
  ADC1.readReg(data, MCP3564_ADR_ADCDATA, 3);
  //Serial.println("ADC Data: ");
  //for(int i = 0; i < 3; ++i) Serial.println(data[i], BIN);

  uint32_t temp_int = 0;
  for (int i = 0; i < 3; ++i)
  {
    temp_int = (temp_int << 8) + data[i];
  }
  float temperature = 0.00040096 * 3.3 * temp_int - 269.13;
  Serial.print("Temperature: ");
  Serial.println(temperature);

  data[0] = B01100011;
  ADC1.writeReg(data, MCP3564_ADR_CONFIG0);
  ADC1.readReg(data, MCP3564_ADR_CONFIG0);
  //Serial.print("Config0: ");
  //Serial.println(data[0], BIN);
  delay(100);
}

void ch1Processing()
{
  //Serial.println("Funciona: ");
  ADC1.readReg(data, MCP3564_ADR_ADCDATA, 3);
  //Serial.println("ADC Data: ");
  //for(int i = 0; i < 3; ++i) Serial.println(data[i], BIN);

  uint32_t prueba_int = 0;
  for (int i = 0; i < 3; ++i)
  {
    prueba_int = (prueba_int << 8) + data[i];
  }
  float prueba = vref * prueba_int * 2 / ADC1.getMaxValue();
  Serial.print("CH1: ");
  Serial.println(prueba);

  data[0] = B01100011;
  ADC1.writeReg(data, MCP3564_ADR_CONFIG0);
  ADC1.readReg(data, MCP3564_ADR_CONFIG0);
  //Serial.print("Config0: ");
  //Serial.println(data[0], BIN);
  delay(100);
}