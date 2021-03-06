#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions for BYTE board
#define LEDS_NUMBER    2

#define LED_1          NRF_GPIO_PIN_MAP(0,13)
#define LED_2          NRF_GPIO_PIN_MAP(0,14)
#define LED_START      LED_1
#define LED_STOP       LED_2

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_1, LED_2 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      NRF_GPIO_PIN_MAP(0,5)
#define BSP_LED_1      NRF_GPIO_PIN_MAP(0,1)

#define BUTTONS_NUMBER 2

#define BUTTON_1       NRF_GPIO_PIN_MAP(0,0)
#define BUTTON_2       NRF_GPIO_PIN_MAP(0,20)
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2

#define HWFC           true

#define SER_CONN_CHIP_RESET_PIN     NRF_GPIO_PIN_MAP(0,1)    // Pin used to reset connectivity chip

// Arduino board mappings
#define BYTE_SCL_PIN             NRF_GPIO_PIN_MAP(1, 1)    //SIO_35 SCL signal pin
#define BYTE_SDA_PIN             NRF_GPIO_PIN_MAP(1, 0)    //SIO_32 SDA signal pin

#define BYTE_EXP_1               NRF_GPIO_PIN_MAP(0, 22)   //SIO_25  
#define BYTE_EXP_2               NRF_GPIO_PIN_MAP(0, 24)   //SIO_24  
#define BYTE_SENSOR_1            NRF_GPIO_PIN_MAP(0, 15)   //SIO_15    
#define BYTE_SENSOR_2            NRF_GPIO_PIN_MAP(0, 17)   //SIO_17 
#define BATT_EN_READ             NRF_GPIO_PIN_MAP(0, 30)   //SIO_30  
#define BATT_READ                NRF_GPIO_PIN_MAP(0, 3)    //SIO_03/AIN2


#ifdef __cplusplus
}
#endif

#endif // PCA10100_H
