#include "sensorDrivers.h"


/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

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



        