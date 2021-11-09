#include "sensorDrivers.h"


/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
uint8_t m_bitShift = 0;
adsGain_t m_gain=GAIN_TWOTHIRDS;

/**
 * @brief TWI initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;
    const nrf_drv_twi_config_t twi_config = {
       .scl                = BYTE_SCL_PIN,
       .sda                = BYTE_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL); 
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

void twi_scanner(void)
{
  ret_code_t err_code;
  uint8_t address;
  uint8_t sample_data;
  bool detected_device = false;
  for (address = 1; address <= TWI_ADDRESSES; address++)
    {
        err_code = nrf_drv_twi_rx(&m_twi, address, &sample_data, sizeof(sample_data));
        if (err_code == NRF_SUCCESS)
        {
            detected_device = true;
            NRF_LOG_INFO("TWI device detected at address 0x%x.", address);
        }
        NRF_LOG_FLUSH();
    }

    if (!detected_device)
    {
        NRF_LOG_INFO("No device was found.");
        NRF_LOG_FLUSH();
    }
}

/**
 * @brief Function for setting PTCA9536B Configuration
 * Writing a 1 will configure the port as input
 * Writing a 0 will configure the port as output
 * Example: 
 *     0x00 - All outputs
 *     0x01 - P0 as input  
 *     0x02 - P1 as input
 */
void PTCA9536_Set_Configuration(uint8_t port)
{
    ret_code_t err_code;
    
    /* Writing to Configuration register (0x03) the port value */
    uint8_t reg[2] = {0x03, port};
    err_code = nrf_drv_twi_tx(&m_twi, PTCA9536B_ADDRESS, reg, sizeof(reg), false);
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Function for setting PTCA9536B Outputs
 */
void PTCA9536_Set_Output(uint8_t port)
{
    ret_code_t err_code;

    /* Writing to Output Port register (0x01) the port value */
    uint8_t reg[2] = {0x01, port};
    err_code = nrf_drv_twi_tx(&m_twi, PTCA9536B_ADDRESS, reg, sizeof(reg), false);
    APP_ERROR_CHECK(err_code);
}


uint16_t ADS1115_Read(uint8_t reg) {
  uint8_t buffer[3];
  buffer[0] = reg;
  APP_ERROR_CHECK(nrf_drv_twi_tx(&m_twi, ADS1115_ADDRESS, buffer, 1, false));
  APP_ERROR_CHECK(nrf_drv_twi_rx(&m_twi, ADS1115_ADDRESS, buffer, 2));
  return ((buffer[0] << 8) | buffer[1]);
}

void ADS1115_Write(uint8_t reg, uint16_t value) {
  uint8_t buffer[3];
  buffer[0] = reg;
  buffer[1] = value >> 8;
  buffer[2] = value & 0xFF;
  APP_ERROR_CHECK(nrf_drv_twi_tx(&m_twi, ADS1115_ADDRESS, buffer, 3, false));
}


int16_t getLastConversionResults(void) {
  // Read the conversion results
  uint16_t res = ADS1115_Read(ADS1X15_REG_POINTER_CONVERT) >> m_bitShift;
  if (m_bitShift == 0) {
    return (int16_t)res;
  } else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}


bool conversionComplete() {
  return (ADS1115_Read(ADS1X15_REG_POINTER_CONFIG) & 0x8000) != 0;
}


int16_t readADC_SingleEnded(uint8_t channel) {
  
  if (channel > 3) {
    return 0;
  }

  // Start with default values
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= 0x0000; ///< +/-6.144V range = Gain 2/3

  // Set data rate
  config |= 0x0080; ///< 128 samples per second (default)

  // Set single-ended input channel
  switch (channel) {
  case (0):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_0;
    break;
  case (1):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_1;
    break;
  case (2):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_2;
    break;
  case (3):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_3;
    break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1X15_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  ADS1115_Write(ADS1X15_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  while (!conversionComplete())
    ;

  // Read the conversion results
  return getLastConversionResults();
}

float computeVolts(int16_t counts) {
  // see data sheet Table 3
  float fsRange;
  switch (m_gain) {
  case GAIN_TWOTHIRDS:
    fsRange = 6.144f;
    break;
  case GAIN_ONE:
    fsRange = 4.096f;
    break;
  case GAIN_TWO:
    fsRange = 2.048f;
    break;
  case GAIN_FOUR:
    fsRange = 1.024f;
    break;
  case GAIN_EIGHT:
    fsRange = 0.512f;
    break;
  case GAIN_SIXTEEN:
    fsRange = 0.256f;
    break;
  default:
    fsRange = 0.0f;
  }
  return counts * (fsRange / (32768 >> m_bitShift));
}