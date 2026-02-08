#include "MCP3564.h"
#include <SPI.h>

#define PIN_CS_1    37
#define PIN_CS_2    31
#define PIN_CS_3    10
#define PIN_CS_4    33

#define PIN_IRQ_1   32
#define PIN_IRQ_2   30
#define PIN_IRQ_3   9
#define PIN_IRQ_4   34

#define PIN_MCLK_1   28
#define PIN_MCLK_2   29
#define PIN_MCLK_3   8
#define PIN_MCLK_4   35

const int polisize = 5; // indica el tamaño del los arrays del coeficientes de k
// Range -270 °C to 0 °C
const float coefs_k_01[polisize]={0.0000000e+00,
                                  1.2329875E-2 ,
                                  -1.4434305E-5 ,
                                  -4.2824995E-9,
                                  -4.2028679E-13};
// Range 0 °C to 1372 °C                   
const float coefs_k_02[polisize]= {0.000000e+00,
                                  2.5132785E-2,
                                  -6.0883423E-8,
                                  5.5358209E-13,
                                  9.3720918E-18};
const uint32_t int24max = 16777216; // 2**24 
const float vref = 3.3; // voltaje de referencia
uint8_t data[4] = {0, 0, 0, 0}; // definimos un array de 4bytes 
volatile float tempTP[14] = {0}; // array que almacena las temperaturas de los termopares
volatile float tempADC[4] = {0}; // array que almacena las temperaturas de los ADC
volatile float thrust = 0; // almacena el valor del empuje

// Muestreo Transductor
uint32_t transducerTime = 0;
uint32_t transducerTimer = 200;

uint32_t serialTime = 0;
uint32_t serialTimer = 1000;

// Declaramos ADCs
auto spiWrapper = &SPI;

union byteToInt
{
  uint8_t bytes[4] = {0};
  int32_t integer;
};

MCP3564 ADC1(spiWrapper, PIN_CS_1, PIN_IRQ_1, PIN_MCLK_1);
MCP3564 ADC2(spiWrapper, PIN_CS_2, PIN_IRQ_2, PIN_MCLK_2);
MCP3564 ADC3(spiWrapper, PIN_CS_3, PIN_IRQ_3, PIN_MCLK_3);
MCP3564 ADC4(spiWrapper, PIN_CS_4, PIN_IRQ_4); // No declaramos el MCLK porque la salida de la teensy no es PWM (mirar librería para más info)

void setup() 
{

  pinMode(A4, INPUT);  // Pin que usamos para el transductor de presión
  // estas líneas de a continuación sirven para inicializar los pines de los adc que no se van a usar, porque no vamos a poner 
  // MCP3564 ADC1(spiWrapper, PIN_CS_i, PIN_IRQ_i) ni adc.begin; así evitamos que entren en conflicto. Cuando si lo usemos para cada ADC
  // podremos borrar las dos lineas, tanto las de pinMode como digitalWrite

  //pinMode(PIN_CS_1, OUTPUT);  // para el spi
  //pinMode(PIN_CS_2, OUTPUT);
  //pinMode(PIN_CS_3, OUTPUT);
  //pinMode(PIN_CS_4, OUTPUT);

  //digitalWrite(PIN_CS_1, true);  // inician en "high" para que no entre con conflicto con el resto, cuando lo llamos pasa a "low"
  //digitalWrite(PIN_CS_2, true);
  //digitalWrite(PIN_CS_3, true);
  //digitalWrite(PIN_CS_4, true);

  SPI.begin();  //  inicializamos la comunicación por SPI
  Serial.begin(230400);  // inicializamos serial a 230400 baudios

  ADC1.begin(ADC1_IRQ, vref); // inicializamos el ADC 1 
  ADC2.begin(ADC2_IRQ, vref); // inicializamos el ADC 2
  ADC3.begin(ADC3_IRQ, vref); // inicializamos el ADC 3 
  ADC4.begin(ADC4_IRQ, vref); // inicializamos el ADC 3 

// Configuro el registro 1
  data[0] = B00100000; //(Pag 92 de la data sheet)
  ADC1.writeReg(data, MCP3564_ADR_CONFIG1);
  data[0] = B00100000; //(Pag 92 de la data sheet)
  ADC2.writeReg(data, MCP3564_ADR_CONFIG1);
  data[0] = B00100000; //(Pag 92 de la data sheet)
  ADC3.writeReg(data, MCP3564_ADR_CONFIG1);
  data[0] = B00100000; //(Pag 92 de la data sheet)
  ADC4.writeReg(data, MCP3564_ADR_CONFIG1);
  delay(100);

// Configuro el registro 2
  data[0] = B10011001; //(Pag 93 de la data sheet)
  ADC1.writeReg(data, MCP3564_ADR_CONFIG2);
  data[0] = B10011001; //(Pag 93 de la data sheet)
  ADC2.writeReg(data, MCP3564_ADR_CONFIG2);
  data[0] = B10011001; //(Pag 93 de la data sheet)
  ADC3.writeReg(data, MCP3564_ADR_CONFIG2);
  data[0] = B10011001; //(Pag 93 de la data sheet)
  ADC4.writeReg(data, MCP3564_ADR_CONFIG2);
  delay(100);

// Configuro el registro 3
  data[0] = B11110000; //(Pag 94 de la data sheet)
  ADC1.writeReg(data, MCP3564_ADR_CONFIG3);
  data[0] = B11110000; //(Pag 94 de la data sheet)
  ADC2.writeReg(data, MCP3564_ADR_CONFIG3);
  data[0] = B11110000; //(Pag 94 de la data sheet)
  ADC3.writeReg(data, MCP3564_ADR_CONFIG3);
  data[0] = B11110000; //(Pag 94 de la data sheet)
  ADC4.writeReg(data, MCP3564_ADR_CONFIG3);
  delay(100);

// Configuro el registro del interrupt (IRQ)
  data[0] = B01110011; //(Pag 95 de la data sheet)
  ADC1.writeReg(data, MCP3564_ADR_IRQ);
  data[0] = B01110011; //(Pag 95 de la data sheet)
  ADC2.writeReg(data, MCP3564_ADR_IRQ);
  data[0] = B01110011; //(Pag 95 de la data sheet)
  ADC3.writeReg(data, MCP3564_ADR_IRQ);
  data[0] = B01110011; //(Pag 95 de la data sheet)
  ADC4.writeReg(data, MCP3564_ADR_IRQ);
  delay(100);

// Configuro el registro del modo SCAN
  uint16_t channelMask = B00011111 << 8; // es realmente 0001111100000000
  uint8_t delay_MCLK = B00000000; // es el delay
  ADC1.setScanSettings(delay_MCLK, channelMask); // mira en la librería para ver como funciona
  ADC2.setScanSettings(delay_MCLK, channelMask); // mira en la librería para ver como funciona
  ADC3.setScanSettings(delay_MCLK, channelMask); // mira en la librería para ver como funciona
  channelMask = B00011011 << 8; // Hay que tener en cuenta que el diferencial CH4-CH5 está desactivado
  ADC4.setScanSettings(delay_MCLK, channelMask); // mira en la librería para ver como funciona
  delay(100);

  // Configuro el registro 0 
  data[0] = B00100011; //(Pag 91 de la data sheet)
  ADC1.writeReg(data, MCP3564_ADR_CONFIG0);
  data[0] = B00100011; //(Pag 91 de la data sheet)
  ADC2.writeReg(data, MCP3564_ADR_CONFIG0);
  data[0] = B00100011; //(Pag 91 de la data sheet)
  ADC3.writeReg(data, MCP3564_ADR_CONFIG0);
  data[0] = B00100011; //(Pag 91 de la data sheet)
  ADC4.writeReg(data, MCP3564_ADR_CONFIG0);
  delay(100);
}


void ADC1_IRQ() { // declara la funcion de lectura del IRQ
  
  byteToInt temp_buffer; //Tambien podría poner {0} para inicializar todo en 0
  int32_t temp = 0;
  uint8_t chID = 0;

  ADC1.readReg(temp_buffer.bytes, MCP3564_ADR_ADCDATA, 4); // leo el registro que almacena los datos 
  chID = (temp_buffer.bytes[0] >> 4) & (B0111); // los bits del chanel ID son los 4 de la izquierda
  
  uint8_t sus; // Cambiamos el orden de los bytes
  sus = temp_buffer.bytes[0];
  temp_buffer.bytes[0] = temp_buffer.bytes[3];
  temp_buffer.bytes[3] = sus;
  sus = temp_buffer.bytes[2];
  temp_buffer.bytes[2] = temp_buffer.bytes[1];
  temp_buffer.bytes[1] = sus;

  if (temp_buffer.bytes[3] % 2 == 1)//~
  { 
    temp_buffer.bytes[3] = B11111111;
  }
  else
  {
    temp_buffer.bytes[3] = 0;
  }
  temp = temp_buffer.integer;

  if (chID == 4)
  {
    tempADC[0] = 0.00040096 * vref * temp - 269.13; // guarda la temperatura del ADC1 en la posición primera del array
  }
  else
  {
    float tempmv = temp * ((vref / int24max) * 1000000) / 4;
    if (temp < 0)
    {
      float tempcels = coefs_k_01[polisize-1];
      for (int i=polisize-2; i>=0; --i)
      {
        tempcels = tempcels*tempmv + coefs_k_01[i]; 
      }
      tempTP[chID] = tempcels + tempADC[0];
    }
    else
    {
      float tempcels = coefs_k_02[polisize-1];
      for (int i=polisize-2; i>=0; --i)
      {
        tempcels = tempcels*tempmv + coefs_k_02[i]; 
      }
      tempTP[chID] = tempcels + tempADC[0];
    }
  }
  /*
    if (chID == 0)
    {
      for (int i = 0; i < 4; ++i)
      {
        Serial.print(temp_buffer.bytes[i],BIN);
        Serial.print(" ");
      }
      Serial.print(temp);
      Serial.println();
    }
  */
} 

void ADC2_IRQ() { // declara la funcion de lectura del IRQ
  
  byteToInt temp_buffer; //Tambien podría poner {0} para inicializar todo en 0
  int32_t temp = 0;
  uint8_t chID = 0;

  ADC2.readReg(temp_buffer.bytes, MCP3564_ADR_ADCDATA, 4); // leo el registro que almacena los datos 
  chID = (temp_buffer.bytes[0] >> 4) & (B0111); // los bits del chanel ID son los 4 de la izquierda
  
  uint8_t sus; // Cambiamos el orden de los bytes
  sus = temp_buffer.bytes[0];
  temp_buffer.bytes[0] = temp_buffer.bytes[3];
  temp_buffer.bytes[3] = sus;
  sus = temp_buffer.bytes[2];
  temp_buffer.bytes[2] = temp_buffer.bytes[1];
  temp_buffer.bytes[1] = sus;

  if (temp_buffer.bytes[3] % 2 == 1) // si tiene signo negativo, saltrá 1 la últiam posición del ultimo byte (no divvisible entre 2)
  { 
    temp_buffer.bytes[3] = B11111111; // en caso de ser negativo cargamos todo ese byte con unos (1)
  }
  else
  {
    temp_buffer.bytes[3] = 0; // si es positiva la medida, cargamos todo ceros(0)
  }
  temp = temp_buffer.integer; 

  if (chID == 4)
  {
    tempADC[1] = 0.00040096 * vref * temp - 269.13; // guarda la temperatura del ADC1 en la posición primera del array
  }
  else
  {
    float tempmv = temp * ((vref / int24max) * 1000000) / 4;
    if (temp < 0)
    {
      float tempcels = coefs_k_01[polisize-1];
      for (int i=polisize-2; i>=0; --i)
      {
        tempcels = tempcels*tempmv + coefs_k_01[i]; 
      }
      tempTP[chID+4] = tempcels + tempADC[1];
    }
    else
    {
      float tempcels = coefs_k_02[polisize-1];
      for (int i=polisize-2; i>=0; --i)
      {
        tempcels = tempcels*tempmv + coefs_k_02[i]; 
      }
      tempTP[chID+4] = tempcels + tempADC[1];
    }
  }
} 

void ADC3_IRQ() { // declara la funcion de lectura del IRQ
  
  byteToInt temp_buffer; //Tambien podría poner {0} para inicializar todo en 0
  int32_t temp = 0;
  uint8_t chID = 0;

  ADC3.readReg(temp_buffer.bytes, MCP3564_ADR_ADCDATA, 4); // leo el registro que almacena los datos 
  chID = (temp_buffer.bytes[0] >> 4) & (B0111); // los bits del chanel ID son los 4 de la izquierda
  
  uint8_t sus; // Cambiamos el orden de los bytes
  sus = temp_buffer.bytes[0];
  temp_buffer.bytes[0] = temp_buffer.bytes[3];
  temp_buffer.bytes[3] = sus;
  sus = temp_buffer.bytes[2];
  temp_buffer.bytes[2] = temp_buffer.bytes[1];
  temp_buffer.bytes[1] = sus;

  if (temp_buffer.bytes[3] % 2 == 1)//~
  { 
    temp_buffer.bytes[3] = B11111111;
  }
  else
  {
    temp_buffer.bytes[3] = 0;
  }
  temp = temp_buffer.integer;

  if (chID == 4)
  {
    tempADC[2] = 0.00040096 * vref * temp - 269.13; // guarda la temperatura del ADC1 en la posición primera del array
  }
  else
  {
    float tempmv = temp * ((vref / int24max) * 1000000) / 4;
    if (temp < 0)
    {
      float tempcels = coefs_k_01[polisize-1];
      for (int i=polisize-2; i>=0; --i)
      {
        tempcels = tempcels*tempmv + coefs_k_01[i]; 
      }
      tempTP[chID+8] = tempcels + tempADC[2];
    }
    else
    {
      float tempcels = coefs_k_02[polisize-1];
      for (int i=polisize-2; i>=0; --i)
      {
        tempcels = tempcels*tempmv + coefs_k_02[i]; 
      }
      tempTP[chID+8] = tempcels + tempADC[2];
    }
  }
} 

void ADC4_IRQ() { // declara la funcion de lectura del IRQ
  
  byteToInt temp_buffer; //Tambien podría poner {0} para inicializar todo en 0
  int32_t temp = 0;
  uint8_t chID = 0;

  ADC4.readReg(temp_buffer.bytes, MCP3564_ADR_ADCDATA, 4); // leo el registro que almacena los datos 
  chID = (temp_buffer.bytes[0] >> 4) & (B0111); // los bits del chanel ID son los 4 de la izquierda
  
  uint8_t sus; // Cambiamos el orden de los bytes
  sus = temp_buffer.bytes[0];
  temp_buffer.bytes[0] = temp_buffer.bytes[3];
  temp_buffer.bytes[3] = sus;
  sus = temp_buffer.bytes[2];
  temp_buffer.bytes[2] = temp_buffer.bytes[1];
  temp_buffer.bytes[1] = sus;

  if (temp_buffer.bytes[3] % 2 == 1)//~
  { 
    temp_buffer.bytes[3] = B11111111;
  }
  else
  {
    temp_buffer.bytes[3] = 0;
  }
  temp = temp_buffer.integer;

  if (chID == 4)
  {
    tempADC[3] = 0.00040096 * vref * temp - 269.13; // guarda la temperatura del ADC1 en la posición primera del array
  }
  else if ((chID == 0)||(chID == 1))
  {
    float tempmv = temp * ((vref / int24max) * 1000000) / 4;
    if (temp < 0)
    {
      float tempcels = coefs_k_01[polisize-1];
      for (int i=polisize-2; i>=0; --i)
      {
        tempcels = tempcels*tempmv + coefs_k_01[i]; 
      }
      tempTP[chID+12] = tempcels + tempADC[3];
    }
    else
    {
      float tempcels = coefs_k_02[polisize-1];
      for (int i=polisize-2; i>=0; --i)
      {
        tempcels = tempcels*tempmv + coefs_k_02[i]; 
      }
      tempTP[chID+12] = tempcels + tempADC[3];
    }
  }
  else// Aquí va la céluala de carga
  {
    thrust = temp; // ya descubriremos como sacar el empuje de aqui
  }
} 


void loop() 
{
  if (micros() > transducerTimer + transducerTime) // esto se llama polling detiene ese bucle no todo el código, el delay detiene todo el código
  {
    transducerTime = micros(); 
    analogRead(A4);
  }

  if (millis() > serialTimer + serialTime) // esto se llama polling detiene ese bucle no todo el código, el delay detiene todo el código
  {
    serialTime = millis();

    for (int i=0; i<4; ++i)
    {
      noInterrupts();
      Serial.print("TemADC_");    
      Serial.print(i+1);
      Serial.print(": "); 
      Serial.print(tempADC[i]);
      Serial.print(" / ");
      interrupts();

      for (int j=i*4; j<i*4+4; ++j)
      {
        if (j < 14)
        {
          noInterrupts();
          Serial.print("TP_");    
          Serial.print(j+1);
          Serial.print(": "); 
          Serial.print(tempTP[j]);
          Serial.print(" / ");
          interrupts();
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
    Serial.print("Transductor:  ");
    Serial.println(analogRead(A4));
    Serial.println("---------------------------------------------------------");
  } 
}
