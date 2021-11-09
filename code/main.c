/*  The BYTE interface firmware
    Copyright (C) 2021  oneohm
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
    http://github.com/oneohm/BYTE_Bluetooth_Firmware
*/

#include "BLE.h"
#include "sensorDrivers.h"

APP_TIMER_DEF(m_battery_timer_id);                                                  /**< Battery timer. */
APP_TIMER_DEF(m_ads_timer_id);                                                      /**< ADS timer. */

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    battery_level_update();
}

static void ads_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    int16_t adcValue[4];
    float voltage[4];
    for(int i=0;i<4;i++){
     adcValue[i] = readADC_SingleEnded(i);
     voltage[i]= computeVolts(adcValue[i]);
     NRF_LOG_INFO("ADC %d:" NRF_LOG_FLOAT_MARKER" v \r\n",i,NRF_LOG_FLOAT(voltage[i]));
    }
    NRF_LOG_FLUSH();
    if((voltage[0]>1)||(voltage[1]>1)||voltage[2]>1||voltage[3]>1){
      int16_t yForceA = (adcValue[0]+adcValue[1])/2-(adcValue[2]+adcValue[3])/2; 
      int16_t xForceA = (adcValue[0]+adcValue[2])/2-(adcValue[1]+adcValue[3])/2;
      // Compute the cartesian coordinated force
      int16_t y45Force = (adcValue[3]-adcValue[0])/2;
      int16_t x45Force = (adcValue[1]-adcValue[2])/2;
      // Compute the cartesian coordinated force
      int16_t yForceB = (y45Force-x45Force)/(0.70710678118);
      int16_t xForceB = (x45Force+y45Force)/(0.70710678118);
      // Average the two computed forces
      int16_t yForce = (yForceA+yForceB)/2;
      int16_t xForce = (xForceA+xForceB)/2;
      mouse_movement_send(xForce, yForce);
    }
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code;

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create battery timer.
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    // Create battery timer.
    err_code = app_timer_create(&m_ads_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                ads_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */
static void timers_start(void)
{
    ret_code_t err_code;

    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_ads_timer_id, ADS_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);

    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    app_sched_execute();
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;

    // Initialize.
    log_init();
    timers_init();
    twi_init();
    twi_scanner();
    //Configure P1 to P3 as inputs, P0 as output
    PTCA9536_Set_Configuration(PTCA_P1|PTCA_P2|PTCA_P3);
    PTCA9536_Set_Output(0x00); //Put all outputs as 

    buttons_leds_init(&erase_bonds);
    power_management_init();
    ble_stack_init();
    scheduler_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init();
    sensor_simulator_init();
    conn_params_init();
    peer_manager_init();

    timers_start();
    advertising_start(erase_bonds);

    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
    }
}


/**
 * @}
 */
