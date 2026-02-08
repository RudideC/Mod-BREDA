#include "MCP3564.h"

// Esto es un constructor de clase, que vale para inicializar la clase. Pone las variables que corresponden, funciones... Solo se ejecuta la primera vez 
// que creas el objeto: "MCP3564 ADC1(spiWrapper, PIN_CS_1, PIN_IRQ_1)"

MCP3564::MCP3564(SPIClass *theSPI, const int8_t pinCS, const int8_t pinIRQ, const int8_t pinMCLK) 
{
// Relacionas el pin del objeto y el del constructor

  _spi     = theSPI;
  _pinCS   = pinCS;
  _pinIRQ  = pinIRQ;
  _pinMCLK = pinMCLK;
};

// Esto inicializa el ADC consus pines y demás movidas

void MCP3564::begin(void (*function)(void), uint32_t mclkfreq, float vref) 
{
  if (_pinIRQ != -1)
  {
    pinMode(_pinIRQ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_pinIRQ), function, FALLING);
  } 

  if (_pinMCLK != -1)
  {
    pinMode(_pinMCLK, OUTPUT);
    analogWriteFrequency(_pinMCLK, mclkfreq);
  } 

  pinMode(_pinCS, OUTPUT);
  digitalWrite(_pinCS, HIGH);

  _spi->begin();

  reset();

  setRef(vref);

  standby();
};

/////////////////////////////////
///       R/W Registers       ///
/////////////////////////////////

void MCP3564::writeReg(uint8_t *data, uint8_t addr, size_t size) 
{
  addr = MCP3564_CMD_IWRITE + (addr << 2); // determina la estructura del registro, consulta data sheet para más info
  _spi->beginTransaction(SPISettings(MCP3564_SPI_SPEED, MCP3564_SPI_ORDER, MCP3564_SPI_MODE)); // define los pines de spi para comunicarse
  noInterrupts();
  digitalWrite(_pinCS, LOW); // al ponerlo en low, estamos seleccionando ese ADC
   _spi->transfer(addr); // indica la dirección del registro a escribir
  _spi->transfer(data, size); // transfiere "data" por spi
  digitalWrite(_pinCS, HIGH); // a la que termina vuelve a poner en high el CS para desactivar ese ADC
  _spi->endTransaction();
  interrupts();
}

// Definimos la función readReg, encargada de leer datos en los registros del ADC (datos que irán al microcontrolador)
// las variables son el array de data (buffer), la dirección del registro en la que está y el tamaño del dato en bytes

void MCP3564::readReg(uint8_t *data, uint8_t addr, size_t size)  
{
  addr = MCP3564_CMD_IREAD + (addr << 2); // determina la estructura del registro, consulta data sheet para más info
  _spi->beginTransaction(SPISettings(MCP3564_SPI_SPEED, MCP3564_SPI_ORDER, MCP3564_SPI_MODE)); // define los pines de spi para comunicarse
  noInterrupts();
  digitalWrite(_pinCS, LOW); // al ponerlo en low, estamos seleccionando ese ADC
  _spi->transfer(addr); // indica la dirección del registro a leer
  _spi->transfer(data, size); // transfiere "data" por spi
  digitalWrite(_pinCS, HIGH); // a la que termina vuelve a poner en high el CS para desactivar ese ADC
  _spi->endTransaction();
  interrupts();
}

/////////////////////////////////
///          Settings         ///
/////////////////////////////////

void MCP3564::setRef(float ref)
{
  _vref = ref;
}

void MCP3564::setReadFormat(uint8_t format)
{
  _readFormat = format;
  if ((format == MCP3564_FORMAT_24) || (format == MCP3564_FORMAT_32_NOSIGN))
  {
    _maxData = MCP3564_MAX_23;
  }
  else if ((format == MCP3564_FORMAT_32_SIGN) || (format == MCP3564_FORMAT_32_ID))
  {
    _maxData = MCP3564_MAX_24;
  }
  uint8_t temp[1] = {0};
  readReg(temp, MCP3564_ADR_CONFIG3);
  temp[0] = (temp[0] && 0b11001111) + (_readFormat << 4);
  writeReg(temp, MCP3564_ADR_CONFIG3);
}

void MCP3564::setConfig0(uint8_t ext_ref, uint8_t clk_sel, uint8_t cs_sel, uint8_t adc_mode)
{
  uint8_t data[1] = {1 << 6};
  data[0] += (ext_ref << 7) + (clk_sel << 4) + (cs_sel << 2) + adc_mode;
  writeReg(data, MCP3564_ADR_CONFIG0);
}

void MCP3564::setConfig1(uint8_t prescaler, uint8_t osr)
{
  uint8_t data[1] = {0};
  data[0] = (prescaler << 6) + (osr << 2);
  writeReg(data, MCP3564_ADR_CONFIG1);
}

void MCP3564::setConfig2(uint8_t boost, uint8_t gain, uint8_t az_mux, uint8_t az_ref)
{
  uint8_t data[1] = {1};
  data[0] += (boost << 6) + (gain << 3) + (az_mux << 2) + (az_ref << 1);
  writeReg(data, MCP3564_ADR_CONFIG2);
}

void MCP3564::setConfig3(uint8_t conversionMode, uint8_t dataFormat, uint8_t crcFormat, uint8_t crcCom, uint8_t offCal, uint8_t gainCal)
{
  _readFormat = dataFormat;
  if ((_readFormat == MCP3564_FORMAT_24) || (_readFormat == MCP3564_FORMAT_32_NOSIGN))
  {
    _maxData = MCP3564_MAX_23;
  }
  else if ((_readFormat == MCP3564_FORMAT_32_SIGN) || (_readFormat == MCP3564_FORMAT_32_ID))
  {
    _maxData = MCP3564_MAX_24;
  }

  uint8_t data[1] = {0};
  data[0] += (conversionMode << 6) + (dataFormat << 4) + (crcFormat << 3) + (crcCom << 2) + (offCal << 1) + gainCal;
  writeReg(data, MCP3564_ADR_CONFIG3);
}

void MCP3564::setIRQ(uint8_t irqMode)
{
  return;
}

void MCP3564::setMux(uint8_t settings)
{
  uint8_t data[1] = {settings};
  writeReg(data, MCP3564_ADR_MUX);
}

void MCP3564::setScanSettings(uint8_t delay, uint16_t channelMask)
{
  uint8_t data[3] = {0};
  data[0] = delay << 5;
  data[1] = channelMask >> 8;
  data[2] = channelMask;
  writeReg(data, MCP3564_ADR_SCAN, 3);
}

void MCP3564::setScanDelayBetweenCycles(uint32_t delay)
{
  uint8_t data[3] = {0};
  data[0] = delay >> 16;
  data[1] = delay >> 8;
  data[2] = delay;
  writeReg(data, MCP3564_ADR_TIMER, 3);
}

/////////////////////////////////
///       Fast Commands       ///
/////////////////////////////////

void MCP3564::fastCommand(uint8_t command)
{
  _spi->beginTransaction(SPISettings(MCP3564_SPI_SPEED, MCP3564_SPI_ORDER, MCP3564_SPI_MODE)); // define los pines de spi para comunicarse
  noInterrupts();
  digitalWrite(_pinCS, LOW); // al ponerlo en low, estamos seleccionando ese ADC
  _spi->transfer(command); // indica la dirección del registro a leer
  digitalWrite(_pinCS, HIGH); // a la que termina vuelve a poner en high el CS para desactivar ese ADC
  _spi->endTransaction();
  interrupts();
}

void MCP3564::fastConversion()
{
  fastCommand(MCP3564_CMD_CONVERSION);
}

void MCP3564::standby()
{
  fastCommand(MCP3564_CMD_STANDBY);
}

void MCP3564::shutdown()
{
  fastCommand(MCP3564_CMD_SHUTDOWN);
}

void MCP3564::fullShutdown()
{
  fastCommand(MCP3564_CMD_FULL_SHUTDOWN);
}

void MCP3564::reset()
{
  fastCommand(MCP3564_CMD_RESET);
}

/////////////////////////////////
///       Read fast ADC       ///
/////////////////////////////////

void MCP3564::readRawADCData(uint8_t *data, size_t size)
{
  readReg(data, MCP3564_ADR_ADCDATA, size);
}

float MCP3564::readADCdata()
{
  uint8_t nBytes = 4;
  uint8_t data[4] = {0};
  if (_readFormat == MCP3564_FORMAT_24) 
  {
    nBytes = 3;
  }
  
  readRawADCData(data, nBytes);
  float volt = _vref;
  switch(_readFormat)
  {
  case MCP3564_FORMAT_24:
  case MCP3564_FORMAT_32_NOSIGN:
    volt *= (((data[0] && 0b01111111) << 16) + (data[1] << 8) + data[2]) * _gain / _maxData;
    if (data[0] >= 0b10000000) 
    {
      volt = -volt;
    }  
    break;
  
  case MCP3564_FORMAT_32_SIGN:
  case MCP3564_FORMAT_32_ID:
    volt *= ((data[1] << 16) + (data[2] << 8) + data[3]) * _gain / _maxData;
    if ((data[0] && 0b00000001) == 1)
    {
      volt = -volt;
    }  
    break;
  }
  return volt;
}