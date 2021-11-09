#ifndef SENSOR_DRV_H
#define SENSOR_DRV_H

#include <stdint.h>
#include <string.h>
#include "nrf_drv_twi.h"
#include "custom_board.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


 /* Number of possible TWI addresses. */
 #define TWI_ADDRESSES      127
 /* TWI instance ID. */
#define TWI_INSTANCE_ID     0

#define PTCA9536B_ADDRESS         0x43
//definitions for configure pins with OR operand
#define PTCA_P0 0x01 
#define PTCA_P1 0x02
#define PTCA_P2 0x04
#define PTCA_P3 0x08

#define ADS1115_ADDRESS

#define DRV2605_ADDRESS

#define MAGNETIC_SENSOR_1_ADDRESS 0X31
#define MAGNETIC_SENSOR_1_ADDRESS 0X32
#define MAGNETIC_SENSOR_1_ADDRESS 0X33
   
  
void twi_init (void);
void twi_scanner(void);

void PTCA9536_Set_Configuration(uint8_t port);
void PTCA9536_Set_Output(uint8_t port);

#endif