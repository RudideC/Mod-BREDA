#ifndef MCP3564_H
#define MCP3564_H

#include <SPI.h>

// Configuración del SPI
#define MCP3564_SPI_ORDER MSBFIRST                       //!< SPI ORDER
#define MCP3564_SPI_MODE  SPI_MODE0                      //!< SPI MODE
#define MCP3564_SPI_ADR   (0b01000000)                   //!< SPI ADDRESS
#define MCP3564_SPI_SPEED (10000000)       

// Referido al modo MUX, solo puedo elegir uno por asi decirlo
#define MCP_OFFSET (0x88)  //!< corresponding mux setting
#define MCP_VCM    (0xF8)  //!< corresponding mux setting
#define MCP_AVDD   (0x98)  //!< corresponding mux setting
#define MCP_TEMP   (0xDE)  //!< corresponding mux setting
#define MCP_DIFFD  (0x67)  //!< corresponding mux setting
#define MCP_DIFFC  (0x45)  //!< corresponding mux setting
#define MCP_DIFFB  (0x23)  //!< corresponding mux setting
#define MCP_DIFFA  (0x01)  //!< corresponding mux setting
#define MCP_CH7    (0x78)  //!< corresponding mux setting
#define MCP_CH6    (0x68)  //!< corresponding mux setting
#define MCP_CH5    (0x58)  //!< corresponding mux setting
#define MCP_CH4    (0x48)  //!< corresponding mux setting
#define MCP_CH3    (0x38)  //!< corresponding mux setting
#define MCP_CH2    (0x28)  //!< corresponding mux setting
#define MCP_CH1    (0x18)  //!< corresponding mux setting
#define MCP_CH0    (0x08)  //!< corresponding mux setting

// Comandos rápidos
#define MCP3564_CMD_CONVERSION    (MCP3564_SPI_ADR | 0b101000)  //!< fast command
#define MCP3564_CMD_STANDBY       (MCP3564_SPI_ADR | 0b101100)  //!< fast command
#define MCP3564_CMD_SHUTDOWN      (MCP3564_SPI_ADR | 0b110000)  //!< fast command
#define MCP3564_CMD_FULL_SHUTDOWN (MCP3564_SPI_ADR | 0b110100)  //!< fast command
#define MCP3564_CMD_RESET         (MCP3564_SPI_ADR | 0b111000)  //!< fast command
#define MCP3564_CMD_SREAD         (MCP3564_SPI_ADR | 0b01)      //!< fast command
#define MCP3564_CMD_IREAD         (MCP3564_SPI_ADR | 0b11)      //!< fast command
#define MCP3564_CMD_IWRITE        (MCP3564_SPI_ADR | 0b10)      //!< fast command

// Direccion de los registros
#define MCP3564_ADR_ADCDATA     0x0   //!< Register ADCDdata address
#define MCP3564_ADR_CONFIG0     0x1   //!< Register Config0 address
#define MCP3564_ADR_CONFIG1     0x2   //!< Register Config1 address
#define MCP3564_ADR_CONFIG2     0x3   //!< Register Config2 address
#define MCP3564_ADR_CONFIG3     0x4   //!< Register Config3 address
#define MCP3564_ADR_IRQ         0x5   //!< Register IRQ address
#define MCP3564_ADR_MUX         0x6   //!< Register MUX address
#define MCP3564_ADR_SCAN        0x7   //!< Register SCAN address
#define MCP3564_ADR_TIMER       0x8   //!< Register Timer address
#define MCP3564_ADR_OFFSET      0x9   //!< Register OFFSET address
#define MCP3564_ADR_GAIN        0xA   //!< Register GAIN address
#define MCP3564_ADR_RESERVED1   0xB   //!< Register
#define MCP3564_ADR_RESERVED2   0xC   //!< Register
#define MCP3564_ADR_LOCK        0xD   //!< Register LOCK address
#define MCP3564_ADR_RESERVED3   0xE   //!< Register
#define MCP3564_ADR_CRCCFG      0xF   //!< Register CRCCFG address

// Resolucion
#define MCP3564_MAX_24        16777216
#define MCP3564_MAX_23        8388608

// OSR
#define MCP3564_OSR_32        0x00
#define MCP3564_OSR_64        0x01
#define MCP3564_OSR_128       0x02
#define MCP3564_OSR_256       0x03
#define MCP3564_OSR_512       0x04
#define MCP3564_OSR_1024      0x05
#define MCP3564_OSR_2048      0x06
#define MCP3564_OSR_4096      0x07
#define MCP3564_OSR_8192      0x08
#define MCP3564_OSR_16384     0x09
#define MCP3564_OSR_20480     0x0A
#define MCP3564_OSR_24576     0x0B
#define MCP3564_OSR_40960     0x0C
#define MCP3564_OSR_49152     0x0D
#define MCP3564_OSR_81920     0x0E
#define MCP3564_OSR_98304     0x0F

// VRef
#define MCP3564_VREF_EXT      0x00
#define MCP3564_VREF_INT      0x01

// Clock Selector
#define MCP3564_CLK_EXT       0x00
#define MCP3564_CLK_INT       0x02
#define MCP3564_CLK_INT_PIN   0x03

// ADC Mode
#define MCP3564_SHUTDOWN      0x00
#define MCP3564_STANDBY       0x02
#define MCP3564_CONVERSION    0x03

// Prescaler
#define MCP3564_PRESCALER_1   0x00
#define MCP3564_PRESCALER_2   0x01
#define MCP3564_PRESCALER_4   0x02
#define MCP3564_PRESCALER_8   0x03

// Boost
#define MCP3564_BOOST_0U5     0x00
#define MCP3564_BOOST_0U66    0x01
#define MCP3564_BOOST_1       0x02
#define MCP3564_BOOST_2       0x03

// Gain
#define MCP3564_GAIN_3U3      0x00
#define MCP3564_GAIN_1        0x01
#define MCP3564_GAIN_2        0x02
#define MCP3564_GAIN_4        0x03
#define MCP3564_GAIN_8        0x04
#define MCP3564_GAIN_16       0x05
#define MCP3564_GAIN_32       0x06
#define MCP3564_GAIN_64       0x07

// Formato de lectura
#define MCP3564_FORMAT_24           0x0
#define MCP3564_FORMAT_32_NOSIGN    0x1
#define MCP3564_FORMAT_32_SIGN      0x2
#define MCP3564_FORMAT_32_ID        0x3

// Espera entre conversion en SCAN
#define MCP3564_DELAY_0       0x0
#define MCP3564_DELAY_8       0x1
#define MCP3564_DELAY_16      0x2
#define MCP3564_DELAY_32      0x3
#define MCP3564_DELAY_64      0x4
#define MCP3564_DELAY_128     0x5
#define MCP3564_DELAY_256     0x6
#define MCP3564_DELAY_512     0x7

class MCP3564 
{
public:
  // Pins
  SPIClass *_spi;
  uint8_t _pinCS;
  uint8_t _pinMCLK;
  uint8_t _pinIRQ;
  
  // Config
  float _vref = 3.3;
  uint8_t _readFormat = MCP3564_FORMAT_24;
  uint32_t _maxData = 16777216;
  uint8_t _gain = 1;

  // Register saved configuration
  uint8_t CONFIG0_data = 0xC0;
  uint8_t CONFIG1_data = 0x0C;   
  uint8_t CONFIG2_data = 0x8B;   
  uint8_t CONFIG3_data = 0x00; 
  uint8_t IRQ_data = 0x73;
  uint8_t MUX_data = 0x01;
  uint8_t SCAN_data[3] = {0x00, 0x00, 0x00};
  uint8_t TIMER_data[3] = {0x00, 0x00, 0x00};
  uint8_t OFFSETCAL_data[3] = {0x00, 0x00, 0x00};
  uint8_t GAINCAL_data[3] = {0x00, 0x00, 0x00};
  uint8_t CRCCFG_data[2] = {0x00, 0x00};

  // ADC data
  uint8_t nReadADCBytes = 3;

  MCP3564(SPIClass *theSPI = &SPI, const int8_t pinCS = SS, const int8_t pinIRQ = -1, const int8_t pinMCLK = -1);
  ~MCP3564() {};
  void begin(void (*function)(void), uint32_t mclkfreq = 20000000, float vref = 3.3);

  // Write and read registers
  void readReg(uint8_t *data, uint8_t addr, size_t size = 1);
  void writeReg(uint8_t *data, uint8_t addr, size_t size = 1);

  // Change settings
  void setRef(float ref);
  void setReadFormat(uint8_t format);
  void setConfig0(uint8_t ext_ref, uint8_t clk_sel, uint8_t cs_sel = 0, uint8_t adc_mode = 0);
  void setConfig1(uint8_t prescaler, uint8_t osr);
  void setConfig2(uint8_t boost, uint8_t gain, uint8_t az_mux = 0, uint8_t az_ref = 1);
  void setConfig3(uint8_t conversionMode, uint8_t dataFormat, uint8_t crcFormat = 0, uint8_t crcCom = 0, uint8_t offCal = 0, uint8_t gainCal = 0);
  void setIRQ(uint8_t);
  void setMux(uint8_t settings);
  void setScanSettings(uint8_t delay, uint16_t channelMask);
  void setScanDelayBetweenCycles(uint32_t delay);

  // Fast commands
  void fastCommand(uint8_t command);
  void fastConversion();
  void standby();
  void shutdown();
  void fullShutdown();
  void reset();

  // Read ADC
  void readRawADCData(uint8_t *data, size_t size = 3);
  float readADCdata();
};

#endif  // MCP3564_H