/** @file
*
* This file contains the source code for Temperature alarm service application
* It also includes the code for Device Information services.
* This application uses the @ref srvlib_conn_params module.
* Adapted from  sample applications provided by Nordic
* 
* Author : Shafy
* Date   : 10/19/2013
* Change log:
* Shafy 				11/20/2013 Added data logger service 
* Hariprasad		11/20/2013 Added device firmware update service
* Shafy         11/27/2013 Increased APP_ADV_INTERVAL to avoid contention with flash erase
*												 Added an additional radio notification handler. The handler for ble_flash is now called from this handler
* Sherin 		    01/20/2014 Changed data logging interval to 15 minutes, included one more page for data logging
* Hariprasad C R 05/01/2014     Chaneged the clock source to LFCLKSRC_RC in ble_stack_init()
*/

#include <stdint.h>
#include <string.h>
#include "ble_bondmngr.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_temp_alarm_service.h"
#include "ble_light_alarm_service.h"
#include "ble_soil_alarm_service.h"
#include "ble_data_log_service.h"
#include "ble_hts.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "ble_stack_handler.h"
#include "app_timer.h"
#include "app_gpiote.h"
#include "app_button.h"
#include "ble_error_log.h"
#include "ble_radio_notification.h"
#include "ble_flash.h"
#include "ble_debug_assert_handler.h"
#include "ble_bas.h"
#include "wimoto_sensors.h"
#include "wimoto.h"
#include "ble_device_mgmt_service.h"
#include "battery.h"

#define SEND_MEAS_BUTTON_PIN_NO              16                                        /**< Button used for sending a measurement. */
#define BONDMNGR_DELETE_BUTTON_PIN_NO        17                                        /**< Button used for deleting all bonded masters during startup. */

#define DEVICE_NAME                          "Wimoto_Grow"                              /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                    "Wimoto"                                   /**< Manufacturer. Will be passed to Device Information Service. */
#define MODEL_NUM                            "Wimoto_Grow"                                   /**< Model number. Will be passed to Device Information Service. */
#define MANUFACTURER_ID                      0x1122334455                               /**< Manufacturer ID, part of System ID. Will be passed to Device Information Service. */
#define ORG_UNIQUE_ID                        0x667788                                   /**< Organizational Unique ID, part of System ID. Will be passed to Device Information Service. */

#define APP_ADV_INTERVAL                     0x81A                                      /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS           0x0000                                     /**< The advertising timeout in units of seconds. */

#define APP_TIMER_PRESCALER                  0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS                 5                                          /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              4                                          /**< Size of timer operation queues. */

#define TEMPERATURE_LEVEL_MEAS_INTERVAL      APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER)/**< temperature level measurement interval (ticks). */
#define CONNECTED_MODE_TIMEOUT_INTERVAL      APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Connected mode timeout interval (ticks). */
#define SECONDS_INTERVAL                     APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER) /**< seconds measurement interval (ticks). */

#define TEMP_TYPE_AS_CHARACTERISTIC          0                                          /**< Determines if temperature type is given as characteristic (1) or as a field of measurement (0). */

#define MIN_CELCIUS_DEGREES                  3688                                       /**< Minimum temperature in celcius for use in the simulated measurement function (multiplied by 100 to avoid floating point arithmetic). */
#define MAX_CELCIUS_DEGRESS                  3972                                       /**< Maximum temperature in celcius for use in the simulated measurement function (multiplied by 100 to avoid floating point arithmetic). */
#define CELCIUS_DEGREES_INCREMENT            36                                         /**< Value by which temperature is incremented/decremented for each call to the simulated measurement function (multiplied by 100 to avoid floating point arithmetic). */

#define MIN_CONN_INTERVAL                    MSEC_TO_UNITS(500, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.5 seconds) */
#define MAX_CONN_INTERVAL                    MSEC_TO_UNITS(1000, UNIT_1_25_MS)          /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                        0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                     MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of indication) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY        APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT         3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_GPIOTE_MAX_USERS                 1                                          /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_DETECTION_DELAY               APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define SEC_PARAM_TIMEOUT                    30                                         /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                       1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                       0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES            BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                        0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE               7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE               16                                         /**< Maximum encryption key size. */

#define FLASH_PAGE_SYS_ATTR                 (BLE_FLASH_PAGE_END - 3)                    /**< Flash page used for bond manager system attribute information. */
#define FLASH_PAGE_BOND                     (BLE_FLASH_PAGE_END - 1)                    /**< Flash page used for bond manager bonding information. */

#define DEAD_BEEF                            0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

static uint16_t                              m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static ble_gap_sec_params_t                  m_sec_params;                              /**< Security requirements for this application. */
static ble_gap_adv_params_t                  m_adv_params;                              /**< Parameters to be passed to the stack when starting advertising. */
static ble_temps_t                           m_temps;                                   /**< Structure used to identify the temperature value service. */
static ble_lights_t                          m_lights;                                  /**< Structure used to identify the light alarm service. */
static ble_soils_t                           m_soils;                                   /**< Structure used to identify the humidity alarm service. */
static ble_device_t                          m_device;                                  /**< Structure used to identify the device management service. */
static ble_dlogs_t													 m_dlogs;																		/**< Structure used to identify the data logger service. */													
static app_timer_id_t                        sensor_meas_timer;                    			/**< sensor measurement timer. */
static app_timer_id_t                        real_time_timer;                           /**< Time keeping timer. */

ble_date_time_t                              m_time_stamp;                              /**< Time stamp. */

uint8_t 																		 battery_level=0; 
ble_bas_t                             			 bas;                                       /**< Structure used to identify the battery service. */

volatile bool 												       m_radio_event = false;                     /**< TRUE if radio is active (or about to become active), FALSE otherwise. */
bool 																				 ENABLE_DATA_LOG=false;											/**< Flag to enable data logger */
bool 																				 READ_DATA=false;
bool 																				 START_DATA_READ=true;											/**< Flag to start data logging*/
bool 																				 TX_COMPLETE=false;
bool                                         CHECK_ALARM_TIMEOUT=false;                 /**< Flag to indicate whether to check for alarm conditions*/
bool                                         DATA_LOG_CHECK=false;
bool                                         TIME_SET = false;                          /**< Flag to indicate user set time*/

extern bool 	                               BROADCAST_MODE;                            /**< Flag used to switch between broadcast and connectable modes*/    
extern bool                                  TEMPS_CONNECTED_STATE;                     /**< This flag indicates temperature service is in connected state or not*/
extern bool                                  LIGHTS_CONNECTED_STATE;                    /**< This flag indicates light service is in connected state or not*/
extern bool                                  SOILS_CONNECTED_STATE;                     /**< This flag indicates humidity service is in connected state or not*/
extern bool  																 DFU_ENABLE;                                /**< This flag indicates DFU mode is enabled/not*/       
extern bool                                  DEVICE_CONNECTED_STATE;                    /**< This flag indicates device management service is in connected state or not*/
extern bool																	 TEMPS_CONNECTED_STATE;											/**< This flag indicates data logger service is in connected state or not*/
extern bool																	 DLOGS_CONNECTED_STATE;

static void device_init(void);
static void temps_init(void);
static void lights_init(void);
static void dis_init(void);
static void soils_init(void);
static void dlogs_init(void);
static void bas_init(void);



/**@brief Function for error handling, which is called when an error has occurred. 
*
* @warning This handler is an example only and does not fit a final product. You need to analyse 
*          how your product is supposed to react in case of error.
*
* @param[in] error_code  Error code supplied to the handler.
* @param[in] line_num    Line number where the handler is called.
* @param[in] p_file_name Pointer to the file name. 
*/
static void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    nrf_gpio_pin_set(ASSERT_LED_PIN_NO);

    // This call can be used for debug purposes during development of an application.
    // @note CAUTION: Activating this code will write the stack to flash on an error.
    //                This function should NOT be used in a final product.
    //                It is intended STRICTLY for development/debugging purposes.
    //                The flash write will happen EVEN if the radio is active, thus interrupting
    //                any communication.
    //                Use with care. Un-comment the line below to use.
    //ble_debug_assert_handler(error_code, line_num, p_file_name);

    // On assert, the system can only recover on reset
    NVIC_SystemReset();
}


/**@brief Callback function for asserts in the SoftDevice.
*
* @details This function will be called in case of an assert in the SoftDevice.
*
* @warning This handler is an example only and does not fit a final product. You need to analyse 
*          how your product is supposed to react in case of Assert.
* @warning On assert from the SoftDevice, the system can only recover on reset.
*
* @param[in]   line_num   Line number of the failing ASSERT call.
* @param[in]   file_name  File name of the failing ASSERT call.
*/
/*static void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
app_error_handler(DEAD_BEEF, line_num, p_file_name);
}*/


/**@brief Function for performing check for the alarm condition.
*/
static void alarm_check(void)
{
    uint32_t err_code;

    err_code = ble_temps_level_alarm_check(&m_temps);    /* Check whether the temperature is out of range*/
    if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
            )
    {
        APP_ERROR_HANDLER(err_code);
    }

    err_code = ble_lights_level_alarm_check(&m_lights);  /* Check whether the light level is out of range*/
    if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
            )
    {
        APP_ERROR_HANDLER(err_code);
    }                       
    err_code = ble_soils_level_alarm_check(&m_soils);    /* Check whether the soil moisture level is out of range*/  
    if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
            )
    {
        APP_ERROR_HANDLER(err_code);
    } 
}

/**@brief Function for performing a Grow Profile parmaters level measurement, and  check for the alarm condition.
*/

static void grow_param_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    static uint8_t minutes_15_count = 0x01, sensor_minutes= 0x01;
    if (minutes_15_count < 0x0F)
    {
        minutes_15_count++;
    }
    else 
    {
        minutes_15_count =0x01;
        DATA_LOG_CHECK=true;
    }

    if (sensor_minutes < 0x02)
    {
        sensor_minutes++;
    }
    else 
    {
        sensor_minutes =0x01;
        CHECK_ALARM_TIMEOUT=true;                           /* Set the flag to indicate alarm conditions check*/
    }


}



/**@brief Function for performing time keeping.
*/
static void real_time_timeout_handler(void * p_context)
{
    uint32_t err_code;
    // Store days in every month in an array
    uint8_t days_in_month[]={0,31,28,31,30,31,30,31,31,30,31,30,31};

    // Check for leap year
    if((m_time_stamp.year% 4 == 0 && m_time_stamp.year%100 != 0) || m_time_stamp.year%400 == 0)
    {
        days_in_month[2] = 29;
    }
    else
    {
        days_in_month[2] = 28;
    }

    // Increment time stamp
    m_time_stamp.seconds += 1;
    if (m_time_stamp.seconds > 59)
    {
        m_time_stamp.seconds -= 60;
        m_time_stamp.minutes++;
        if (m_time_stamp.minutes > 59)
        {
            m_time_stamp.minutes = 0;
            m_time_stamp.hours++;
            if (m_time_stamp.hours > 23)
            {
                m_time_stamp.hours = 0;
                m_time_stamp.day++;
                if (m_time_stamp.day > (days_in_month[m_time_stamp.month]))	
                {
                    m_time_stamp.day = 1;
                    m_time_stamp.month++;
                    if (m_time_stamp.month > 12)	
                    {
                        m_time_stamp.month = 1;
                        m_time_stamp.year++;
                    }
                }
            }
        }
    }

    // Update new time to the user 
    err_code= ble_time_update(&m_device, &m_time_stamp);

    if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
            )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/**@brief Function for the Timer initialization.
*
* @details Initializes the timer module. This creates and starts application timers.
*/
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create timers
    err_code = app_timer_create(&sensor_meas_timer,    /* Timer for grow parameters measurement time-out*/
    APP_TIMER_MODE_REPEATED,
    grow_param_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&real_time_timer,      /* Timer for grow parameters measurement time-out*/
    APP_TIMER_MODE_REPEATED,
    real_time_timeout_handler);
    APP_ERROR_CHECK(err_code);	 
} 


/**@brief Function for starting application timers.
*/
static void application_timers_start(void)
{
    uint32_t err_code;

    // Start application timers
    err_code = app_timer_start(sensor_meas_timer, TEMPERATURE_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(real_time_timer, SECONDS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

}


/**@brief Function for the GAP initialization.
*
* @details This function shall be used to setup all the necessary GAP (Generic Access Profile)
*          parameters of the device. It also sets the permissions and appearance.
*/
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_THERMOMETER);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
*
* @details Encodes the required advertising data and passes it to the stack.
*          Also builds a structure to be passed to the stack when starting advertising.
*/
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t advdata2;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    // Base UUID of Grow Profile
    static uint8_t Vendor_Spec_Uuid[16] = {0xDA, 0xF4, 0x47, 0x06, 0xBF, 0xB0, 0x4D, 0xD8, 0x92, 0x93, 0x62, 0xAF, 0x5F, 0x54, 0x5E, 0x31};
    ble_advdata_manuf_data_t   manuf_data;

    // Setting up base UUID as manufacturing data
    manuf_data.company_identifier = COMPANY_IDENTIFER;                                  /* Company Identifier */
    manuf_data.data.size          = 16;
    manuf_data.data.p_data        = Vendor_Spec_Uuid;  

    ble_uuid_t adv_uuids[] = 
    {
        {GROW_PROFILE_TEMP_SERVICE_UUID,									BLE_UUID_TYPE_BLE}, 
        {GROW_PROFILE_LIGHTS_SERVICE_UUID, 							  BLE_UUID_TYPE_BLE},
        {GROW_PROFILE_SOILS_SERVICE_UUID, 								BLE_UUID_TYPE_BLE}, 								
    };

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags.size              = sizeof(flags);
    advdata.flags.p_data            = &flags;
    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = adv_uuids;

    memset(&advdata2, 0, sizeof(advdata2));

    advdata2.name_type               = BLE_ADVDATA_NO_NAME;
    advdata2.include_appearance      = false;
    advdata2.flags.size              = 0;
    advdata2.p_manuf_specific_data   = &manuf_data; 

    err_code = ble_advdata_set(&advdata, &advdata2);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising)
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.p_peer_addr = NULL;                           
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;

}


/**@brief Function for initializing services that will be used by the application.
*
* @details Initialize the Temperature alarm and Device Information services.
*/
static void services_init(void)
{
    temps_init();         /* Initialize temperature alarm service*/
    lights_init();        /* Initialize light alarm service*/ 
    soils_init();         /* Initialize soil alarm service*/
    dlogs_init();				  /* Initialize the data logger service*/
    device_init();        /* Initialize device management service*/
    dis_init();           /* Initialize device information service*/	
    bas_init();	          /* Initialize battery service*/
}

/**@brief Function for initializing battery service
*/
static void bas_init(void)
{

    uint32_t       err_code;
    ble_bas_init_t bas_init;
    // Initialize Battery Service
    memset(&bas_init, 0, sizeof(bas_init));

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;     /* 100% battery level */

    err_code = ble_bas_init(&bas, &bas_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing Device management alarm service*/
static void device_init(void)
{

    uint32_t         err_code;
    ble_device_init_t   device_init;	

    // Initialize temperature range Service
    memset(&device_init, 0, sizeof(device_init));

    // Here the sec level for the Device management Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&device_init.device_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&device_init.device_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&device_init.device_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&device_init.device_char_attr_md2.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&device_init.battery_level_report_read_perm);

    device_init.evt_handler          = NULL;
    device_init.write_evt_handler    = NULL;
    device_init.support_notification = true;
    device_init.p_report_ref         = NULL; 

    // Set the default low value for DFU and Switch mode characteristics 
    device_init.device_dfu_mode_set          = DEFAULT_ALARM_SET;
    device_init.device_mode_switch_set       = DEFAULT_ALARM_SET;

    device_init.device_time_stamp_set[0] =  0x00;
    device_init.device_time_stamp_set[1] =	0x00;	
    device_init.device_time_stamp_set[2] =  0x00;
    device_init.device_time_stamp_set[3] =	0x00;	
    device_init.device_time_stamp_set[4] =  0x00;
    device_init.device_time_stamp_set[5] =	0x00;	
    device_init.device_time_stamp_set[6] =  0x00;

    err_code = ble_device_init(&m_device, &device_init);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing temperature alarm service*/
static void temps_init(void)
{

    uint32_t         err_code;
    ble_temps_init_t   temps_init;
    // Initialize temperature alarm Service
    memset(&temps_init, 0, sizeof(temps_init));

    // Here the sec level for the temperature Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&temps_init.temperature_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&temps_init.temperature_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&temps_init.temperature_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&temps_init.temperature_char_attr_md2.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&temps_init.battery_level_report_read_perm);

    temps_init.evt_handler          = NULL;
    temps_init.write_evt_handler    = NULL;
    temps_init.support_notification = true;
    temps_init.p_report_ref         = NULL; 

    // Set the default low value and high value of temperature alarm
    temps_init.temperature_low_level[0]    = TEMP_DEFAULT_LOW_VALUE_LOWER_BYTE;
    temps_init.temperature_low_level[1]    = TEMP_DEFAULT_LOW_VALUE_HIGHER_BYTE;
    temps_init.temperature_high_level[0]   = TEMP_DEFAULT_HIGH_VALUE_LOWER_BYTE;
    temps_init.temperature_high_level[1]   = TEMP_DEFAULT_HIGH_VALUE_HIGHER_BYTE;
    temps_init.temperature_alarm_set       = DEFAULT_ALARM_SET;
    temps_init.temperature_alarm           = RESET_ALARM;

    err_code = ble_temps_init(&m_temps, &temps_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing light alarm service*/
static void lights_init(void)
{

    uint32_t         err_code;
    ble_lights_init_t   lights_init;

    // Initialize light alarm Service
    memset(&lights_init, 0, sizeof(lights_init));

    // Here the sec level for the Light Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&lights_init.lights_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&lights_init.lights_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&lights_init.lights_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&lights_init.lights_char_attr_md2.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&lights_init.battery_level_report_read_perm);

    lights_init.evt_handler          = NULL;
    lights_init.write_evt_handler    = NULL;
    lights_init.support_notification = true;
    lights_init.p_report_ref         = NULL; 

    // Set the default low value and high value of Light alarm
    lights_init.light_low_value[0]    = LIGHT_DEFAULT_LOW_VALUE_LOWER_BYTE;
    lights_init.light_low_value[1]    = LIGHT_DEFAULT_LOW_VALUE_HIGHER_BYTE;
    lights_init.light_high_value[0]   = LIGHT_DEFAULT_HIGH_VALUE_LOWER_BYTE;
    lights_init.light_high_value[1]   = LIGHT_DEFAULT_HIGH_VALUE_HIGHER_BYTE;
    lights_init.light_alarm_set       = DEFAULT_ALARM_SET;
    lights_init.light_alarm           = RESET_ALARM;

    err_code = ble_lights_init(&m_lights, &lights_init);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for initializing soil moisture alarm service*/
static void soils_init(void)
{
    uint32_t           err_code;
    ble_soils_init_t   soils_init;

    // Initialize humidity alarm Service
    memset(&soils_init, 0, sizeof(soils_init));

    // Here the sec level for the soil moisture Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&soils_init.soil_mois_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&soils_init.soil_mois_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&soils_init.soil_mois_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&soils_init.soil_mois_char_attr_md2.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&soils_init.soil_mois_level_report_read_perm);

    soils_init.evt_handler          = NULL;
    soils_init.write_evt_handler    = NULL;
    soils_init.support_notification = true;
    soils_init.p_report_ref         = NULL; 

    // Set the default low value and high value of humidity level
    soils_init.soil_mois_low_value    = SOIL_MOIS_DEFAULT_LOW_VALUE;
    soils_init.soil_mois_high_value   = SOIL_MOIS_DEFAULT_HIGH_VALUE;
    soils_init.soil_mois_alarm_set    = DEFAULT_ALARM_SET;
    soils_init.soil_mois_alarm        = RESET_ALARM;

    err_code = ble_soils_init(&m_soils, &soils_init);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for initializing data logger service*/
static void dlogs_init(void)
{

    uint32_t           err_code;
    ble_dlogs_init_t   dlogs_init;

    // Initialize humidity alarm Service
    memset(&dlogs_init, 0, sizeof(dlogs_init));

    // Here the sec level for the soil moisture Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dlogs_init.dlogs_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dlogs_init.dlogs_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dlogs_init.dlogs_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dlogs_init.dlogs_char_attr_md2.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dlogs_init.dlogs_report_read_perm);

    dlogs_init.evt_handler          = NULL;
    dlogs_init.write_evt_handler    = NULL;
    dlogs_init.support_notification = true;
    dlogs_init.p_report_ref         = NULL; 
    dlogs_init.data_logger_enable   = DEFAULT_ALARM_SET;
    dlogs_init.read_data_switch     = DEFAULT_ALARM_SET;

    // Set the default low value and high value of humidity level

    err_code = ble_dlogs_init(&m_dlogs, &dlogs_init);
    APP_ERROR_CHECK(err_code);

}
/**@brief Function for initializing device information service*/
static void dis_init(void )
{

    uint32_t         err_code;
    ble_dis_init_t   dis_init;
    ble_dis_sys_id_t sys_id;


    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init.model_num_str,     MODEL_NUM);

    sys_id.manufacturer_id            = MANUFACTURER_ID;
    sys_id.organizationally_unique_id = ORG_UNIQUE_ID;
    dis_init.p_sys_id                 = &sys_id;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing security parameters.
*/
static void sec_params_init(void)
{
    m_sec_params.timeout      = SEC_PARAM_TIMEOUT;
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;  
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}


/**@brief Function for starting advertising.
*/
static void advertising_start(void)
{
    uint32_t err_code;
    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);

    //    nrf_gpio_pin_set(ADVERTISING_LED_PIN_NO);
}


/**@brief Function for handling the Connection Parameters Module.
*
* @details This function will be called for all events in the Connection Parameters Module which
*          are passed to the application.
*          @note All this function does is to disconnect. This could have been done by simply
*                setting the disconnect_on_fail config parameter, but instead we use the event
*                handler mechanism to demonstrate its use.
*
* @param[in]   p_evt   Event received from the Connection Parameters Module.
*/
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
*
* @param[in]   nrf_error   Error code containing information about what went wrong.
*/
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
*/
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Application's BLE Stack events.
*
* @param[in]   p_ble_evt   Bluetooth stack event.
*/
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        nrf_gpio_pin_set(CONNECTED_LED_PIN_NO);
        nrf_gpio_pin_clear(ADVERTISING_LED_PIN_NO);
        // Start detecting button presses
        err_code = app_button_enable();
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        nrf_gpio_pin_clear(CONNECTED_LED_PIN_NO);

        m_conn_handle               = BLE_CONN_HANDLE_INVALID;

        // Stop detecting button presses when not connected
        err_code = app_button_disable();
        APP_ERROR_CHECK(err_code);

        // Since we are not in a connection and have not started advertising, store bonds
        err_code = ble_bondmngr_bonded_masters_store();
        APP_ERROR_CHECK(err_code);

        advertising_start();
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        err_code = sd_ble_gap_sec_params_reply(m_conn_handle, 
        BLE_GAP_SEC_STATUS_SUCCESS, 
        &m_sec_params);
        break;

    case BLE_GAP_EVT_TIMEOUT:
        if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT)
        {
            nrf_gpio_pin_clear(ADVERTISING_LED_PIN_NO);

            // Go to system-off mode (this function will not return; wakeup will cause a reset).
            GPIO_WAKEUP_BUTTON_CONFIG(SEND_MEAS_BUTTON_PIN_NO);
            GPIO_WAKEUP_BUTTON_CONFIG(BONDMNGR_DELETE_BUTTON_PIN_NO);

            err_code = sd_power_system_off();    
        }
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        if (p_ble_evt->evt.gatts_evt.params.timeout.src == BLE_GATT_TIMEOUT_SRC_PROTOCOL)
        {
            err_code = sd_ble_gap_disconnect(m_conn_handle,
            BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        }
        break;
    case BLE_EVT_TX_COMPLETE:
        TX_COMPLETE=true;
        break;

    default:
        break;
    }

    APP_ERROR_CHECK(err_code);
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
*
* @details This function is called from the BLE Stack event interrupt handler after a BLE stack
*          event has been received.
*
* @param[in]   p_ble_evt   Bluetooth stack event.
*/
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{

    ble_temps_on_ble_evt(&m_temps, p_ble_evt);
    ble_lights_on_ble_evt(&m_lights, p_ble_evt);
    ble_soils_on_ble_evt(&m_soils, p_ble_evt);
    ble_bas_on_ble_evt(&bas, p_ble_evt);
    ble_dlogs_on_ble_evt(&m_dlogs, p_ble_evt);
    ble_device_on_ble_evt(&m_device, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_bondmngr_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
}


/**@brief Function for initializing the BLE stack.
*
* @details Initializes the SoftDevice and the BLE event interrupt.
*/
static void ble_stack_init(void)
{
    BLE_STACK_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_2000MS_CALIBRATION,
    BLE_L2CAP_MTU_DEF,
    ble_evt_dispatch,
    false);
}


/**@brief Function for handling button events.
*
* @param[in]   pin_no   The pin number of the button pressed.
*/
static void button_event_handler(uint8_t pin_no)
{
    switch (pin_no)
    {
    case SEND_MEAS_BUTTON_PIN_NO:
        //   temperature_measurement_send();
        break;

    default:
        APP_ERROR_HANDLER(pin_no);
    }
}


/**@brief Function for initializing the GPIOTE handler module.
*/
static void gpiote_init(void)
{
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}


/**@brief Function for initializing button module.
*/
static void buttons_init(void)
{
    static app_button_cfg_t buttons[] =
    {
        {SEND_MEAS_BUTTON_PIN_NO,       false, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {BONDMNGR_DELETE_BUTTON_PIN_NO, false, NRF_GPIO_PIN_NOPULL, NULL}
    };

    APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);
}


/**@brief Function for handling a Bond Manager error.
*
* @param[in]   nrf_error   Error code containing information about what went wrong.
*/
static void bond_manager_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the Bond Manager events.
*
* @param[in]   p_evt   Data associated to the bond manager event.
*/
static void bond_evt_handler(ble_bondmngr_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
    case BLE_BONDMNGR_EVT_ENCRYPTED:
        // Send a single temperature measurement if indication is enabled.
        // NOTE: For this to work, make sure ble_hts_on_ble_evt() is called before
        //       ble_bondmngr_on_ble_evt() in ble_evt_dispatch().
        //err_code = ble_hts_is_indication_enabled(&m_hts, &is_indication_enabled);
        //APP_ERROR_CHECK(err_code);
        /*if (is_indication_enabled)
{
temperature_measurement_send();
}*/
        break;

    default:
        break;
    }
}


/**@brief Function for the Bond Manager initialization.
*/
static void bond_manager_init(void)
{
    uint32_t            err_code;
    ble_bondmngr_init_t bond_init_data;
    bool                bonds_delete;

    // Clear all bonded masters if the Bonds Delete button is pushed
    err_code = app_button_is_pushed(BONDMNGR_DELETE_BUTTON_PIN_NO, &bonds_delete);
    APP_ERROR_CHECK(err_code);

    // Initialize the Bond Manager
    bond_init_data.flash_page_num_bond     = FLASH_PAGE_BOND;
    bond_init_data.flash_page_num_sys_attr = FLASH_PAGE_SYS_ATTR;
    bond_init_data.evt_handler             = bond_evt_handler;
    bond_init_data.error_handler           = bond_manager_error_handler;
    bond_init_data.bonds_delete            = bonds_delete;

    err_code = ble_bondmngr_init(&bond_init_data);
    APP_ERROR_CHECK(err_code);
}

/**@brief Radio Notification event handler.
*/
void radio_active_evt_handler(bool radio_active)
{
    m_radio_event = radio_active;
    ble_flash_on_radio_active_evt(m_radio_event);									/*call the event handler */
}


/**@brief Function for initializing the Radio Notification events.
*/
static void radio_notification_init(void)
{
    uint32_t err_code;

    err_code = ble_radio_notification_init(NRF_APP_PRIORITY_HIGH,
    NRF_RADIO_NOTIFICATION_DISTANCE_4560US,
    radio_active_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Power manager.
*/
static void power_manage(void)
{
    uint32_t err_code = sd_app_event_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for creating log data.
*/
static void create_log_data(uint32_t * data)
{
    uint16_t current_temperature;
    uint16_t current_light_level;
    uint16_t current_soil_mois_level;

    current_temperature=read_temperature();					
    current_light_level=read_light_level();
    current_soil_mois_level=read_soil_mois_level();

    data[0]=(m_time_stamp.year<<16)|(m_time_stamp.month<<8)|m_time_stamp.day;				 /* First word written to memory contains date (YYYYMMDD)*/
    data[1]=(m_time_stamp.hours<<16)|(m_time_stamp.minutes<<8)|m_time_stamp.seconds; /* Second word contains time HHMMSS*/

    data[2]=current_temperature;
    data[2]=(current_temperature<<16)|current_light_level;										       /* Third word contains temperature and light level*/	
    data[3]=current_soil_mois_level;                                                 /* Fourth word contains soil moisture level*/
}

/**@brief Function for checking whether to log data.
*/
static void data_log_check()
{
    uint32_t log_data[4];                                 /* Array storing the data to be logged */

    if(ENABLE_DATA_LOG && !READ_DATA)									    /* If enabled, start data logging functionality*/
    {   
        create_log_data(log_data);                        /* Create the data to be logged */
        write_data_flash(log_data);	                      /* Log the data to flash */
    }
}
/**@brief Function for application main entry.
*/
void connectable_mode(void)
{
    uint32_t err_code;
    // Initialize.
    ble_stack_init();
    twi_master_init();                    /* Configure twi*/
    config_tmp102_shutdown_mode();        /* Configure tmp102 in shut-down mode*/
    ISL29023_config_FSR_and_powerdown();  /* Configure isl29023 */
    adc_init();												  	/* Initialize ADC for interfacing soil moisture sensor*/
    timers_init();
    gpiote_init();
    buttons_init();
    bond_manager_init();
    gap_params_init();
    advertising_init();
    services_init();
    conn_params_init();
    sec_params_init();
    radio_notification_init();
    twi_turn_OFF();
    application_timers_start();    
    advertising_start();

    // Enter main loop.
    for (;;)
    {

        if((BROADCAST_MODE) && (!TEMPS_CONNECTED_STATE) && (!LIGHTS_CONNECTED_STATE) && (!SOILS_CONNECTED_STATE)&&(!DLOGS_CONNECTED_STATE)) /*If the broadcast mode flag is true and services are not connected stop advertising and exit*/
        {                          
            sd_ble_gap_adv_stop();			                      /* Stop advertising */
            break;
        }

        if(DFU_ENABLE && (!DEVICE_CONNECTED_STATE) && (!TEMPS_CONNECTED_STATE) && (!LIGHTS_CONNECTED_STATE) && (!SOILS_CONNECTED_STATE)) /*If the dfu enable flag is true and services are not connected go to the bootloader*/ 
        {
            sd_power_gpregret_set(1);                         /* If DFU mode is enabled , set the value of general purpose retention register to 1*/
            sd_nvic_SystemReset();                            /* Apply a system reset for jumping into bootloader*/
        }

        if (DATA_LOG_CHECK)
        {
            data_log_check();
            DATA_LOG_CHECK= false;
        }

        if(READ_DATA)																			   /*If enabled, start sending data to the connected device*/
        {
            err_code=app_timer_stop(sensor_meas_timer);		     /* Stop the timers before start sending the historical data*/
            APP_ERROR_CHECK(err_code);
            READ_DATA=false;
            ENABLE_DATA_LOG = false;                           /* Disable data logging functionality */
            send_data(&m_dlogs);															 /* Start sending the data*/												
            application_timers_start();												 /* Restart the timers when sending is finished*/
            err_code=reset_data_log(&m_dlogs);								 /* Reset the data logger enable and data read switches*/
            APP_ERROR_CHECK(err_code);	
        }
        if(TIME_SET)
        {
            create_time_stamp(&m_device, &m_time_stamp);     /* Create new time stamp from user set time*/
            TIME_SET = false;                                /* Reset the flag*/

        }
        if (CHECK_ALARM_TIMEOUT)                             /*Check for sensor measurement time-out*/
        {
            alarm_check();                                   /* Checks for alarm in all services*/
            battery_start();		                             /* Measure battery level*/    
            CHECK_ALARM_TIMEOUT=false;                       /* Reset the flag*/
        }
        power_manage();             												 /* Switch to a low power state*/

    }
}


// * @}

