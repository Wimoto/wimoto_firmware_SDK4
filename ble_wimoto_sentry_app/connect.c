/** @file
*
* This file contains the source code for sentry alarm service application
* It also includes the code for Device Information services.
* This application uses the @ref srvlib_conn_params module.
* Adapted from  sample applications provided by Nordic
* 
* Author : Haiprasad C R
* Date   : 11/29/2013
*
* Change log:
* 
* Sherin    		 11/25/2013     Added Data logger service to the profile
* Sherin 		     01/20/2014     Changed datalogging interval to 15 minutes, included one more page for data logging
* Hariprasad C R 05/01/2014     Chaneged the clock source to LFCLKSRC_RC in ble_stack_init()
*/
#define BOARD_PCA10001											 pca10001
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
#include "ble_pir_alarm_service.h"
#include "ble_accelerometer_alarm_service.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "battery.h"

#define SEND_MEAS_BUTTON_PIN_NO              16                                         /**< Button used for sending a measurement. */
#define BONDMNGR_DELETE_BUTTON_PIN_NO        17                                         /**< Button used for deleting all bonded masters during startup. */

#define DEVICE_NAME                          "Wimoto_Sentry"                             /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                    "Wimoto"                                   /**< Manufacturer. Will be passed to Device Information Service. */
#define MODEL_NUM                            "Wimoto"                                   /**< Model number. Will be passed to Device Information Service. */
#define MANUFACTURER_ID                      0x1122334455                               /**< Manufacturer ID, part of System ID. Will be passed to Device Information Service. */
#define ORG_UNIQUE_ID                        0x667788                                   /**< Organizational Unique ID, part of System ID. Will be passed to Device Information Service. */

#define APP_ADV_INTERVAL                     0x81A                                      /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS           0x0000                                     /**< The advertising timeout in units of seconds. */

#define APP_TIMER_PRESCALER                  0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS                 5                                          /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              4                                          /**< Size of timer operation queues. */

#define SENTRY_LEVEL_MEAS_INTERVAL           APP_TIMER_TICKS(60000, APP_TIMER_PRESCALER)/**< sentry level measurement interval (ticks). */
#define CONNECTED_MODE_TIMEOUT_INTERVAL      APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Connected mode timeout interval (ticks). */
#define SECONDS_INTERVAL                     APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER) /**< seconds measurement interval (ticks). */

#define WATER_TYPE_AS_CHARACTERISTIC         0                                          /**< Determines if water type is given as characteristic (1) or as a field of measurement (0). */

#define MIN_CONN_INTERVAL                    MSEC_TO_UNITS(500, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.5 seconds) */
#define MAX_CONN_INTERVAL                    MSEC_TO_UNITS(1000, UNIT_1_25_MS)          /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                        0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                     MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of indication) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY        APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update . */
#define MAX_CONN_PARAMS_UPDATE_COUNT         3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_GPIOTE_MAX_USERS                 4                                          /**< Maximum number of users of the GPIOTE handler. */

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

volatile bool                                m_radio_event = false;                 

uint8_t                                      movement_gpio_pin_val;
static uint16_t                              m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static ble_gap_sec_params_t                  m_sec_params;                              /**< Security requirements for this application. */
static ble_gap_adv_params_t                  m_adv_params;                              /**< Parameters to be passed to the stack when starting advertising. */
static ble_dlogs_t                           m_dlogs;																	  /**< Structure used to identify the data logger service. */
static ble_device_t                          m_device;                                  /**< Structure used to identify the Device management service. */
static ble_pir_t                             m_pir;                                     /**< Structure used to identify the Passive Infrared alarm service. */
static ble_movement_t                        m_movement;                                /**< Structure used to identify the Aceelerometer alarm service. */
static app_timer_id_t                        sentry_measurement_timer;                  /**< Sentry profile measurement timer. */
static app_timer_id_t                        real_time_timer;                           /**< Time keeping timer. */

app_gpiote_user_id_t 								         pir_measurement_gpiote;                    /**< PIR presence measurement gpiote. */
app_gpiote_user_id_t 	                       movement_measurement_gpiote;               /**< Movement presence measurement gpiote. */
ble_date_time_t                              m_time_stamp;                              /**< Time stamp. */
uint8_t 																		 battery_level=0;                           /**< Battery level variable */
ble_bas_t                             			 bas;                                       /**< Structure used to identify the battery service. */

bool 																				 ENABLE_DATA_LOG = false;										/**< Flag to enable data logger */
bool 																				 READ_DATA       = false;
bool 																				 START_DATA_READ = true;									  /**< flag to start data logging */
bool 																				 TX_COMPLETE     = false;
bool                                         TIME_SET          = false;                 /**< Flag to start time updation */
bool                                         PIR_EVENT_FLAG = false;                    /**< Flag to indicate the event occurred on GPIO pin configured for PIR sensor */  
bool                                         MOVEMENT_EVENT_FLAG = false;               /**< Event occurred on Movement/accelerometer interrupt pin */
bool                                         CHECK_ALARM_TIMEOUT=false;                 /**< Flag to indicate whether to check for alarm conditions*/
bool                                         DATA_LOG_CHECK=false;
bool                                         CLEAR_MOVE_ALARM=false;

extern bool 	                               BROADCAST_MODE;                            /**< Flag used to switch between broadcast and connectable modes */    
extern bool																	 DLOGS_CONNECTED_STATE;                     /**< Specifies data logger service is connected or not */
extern bool  																 DFU_ENABLE;                                /**< This flag indicates DFU mode is enabled/not */       
extern bool                                  DEVICE_CONNECTED_STATE;                    /**< This flag indicates device management service is in connected start or now */
extern bool     	                           PIR_CONNECTED_STATE;                       /**< Flag indicates Passive INfrared alarm service is in connected start or now */
extern bool                                  ACCELEROMETER_CONNECTED_STATE;             /**< Flag indicates accelerometer alarm service is in connected start or now */ 
extern uint8_t  current_xyz_array[3];


static void device_init(void);
static void dlogs_init(void);
static void dis_init(void);
static void pir_init(void);
static void movement_init(void);
static void bas_init(void);

/**@brief Function for error handling, which is called when an error has occurred. 
*
* @warning This handler is an example only and does not fit a final product. You need to analyze 
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



///**@brief Function for performing a sentry parameters monitoring .
// */
//static void alarm_check(void)
//{
// 
//		
//}


/**@brief Function for performing a Sentry Profile parameters level measurement, and  check for the alarm condition.
*/
static void sentry_param_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    static uint8_t minutes_count = 0x01;
    if (minutes_count < 0x0F)
    {
        minutes_count++;
    }
    else 
    {
        minutes_count =0x01;
        DATA_LOG_CHECK=true;
    }
    battery_start();                              /* Updates the battery level*/
}


/**@brief Function for performing time keeping.
*/
static void real_time_timeout_handler(void * p_context)
{
    uint32_t err_code;
    uint8_t days_in_month[]={0,31,28,31,30,31,30,31,31,30,31,30,31};


    if((m_time_stamp.year% 4 == 0 && m_time_stamp.year%100 != 0) || m_time_stamp.year%400 == 0)
    {
        days_in_month[2] = 29;
    }
    else
    {
        days_in_month[2] = 28;
    }

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
                if (m_time_stamp.day > (days_in_month[m_time_stamp.month]))	/* Increment day till days_in_month[time_stamp.month]*/
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
    err_code = app_timer_create(&sentry_measurement_timer, /*Timer for sentry  measurement time-out*/
    APP_TIMER_MODE_REPEATED,
    sentry_param_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);


    // Create timer for time keeping
    err_code = app_timer_create(&real_time_timer,          /*Timer for climate parameters measurement time out*/
    APP_TIMER_MODE_REPEATED,
    real_time_timeout_handler);
    APP_ERROR_CHECK(err_code);

} 

/**@brief Function for starting application timers.
*/
static void application_timers_start(void)
{
    uint32_t err_code;

    // Start measurement time-out timer
    err_code = app_timer_start(sentry_measurement_timer, SENTRY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    // Start the time keeping timer
    err_code = app_timer_start(real_time_timer, SECONDS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code); 

}


/**@brief Function for the GAP initialization.
*
* @details This function shall be used to set-up all the necessary GAP (Generic Access Profile)
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
    ble_advdata_t advdata,advdata2;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    // Base UUID of Sentry Profile
    static uint8_t Vendor_Spec_Uuid[16] = {0x42, 0x09, 0xDC, 0x67, 0xE4, 0x33, 0x44, 0x20, 0x83, 0xD8, 0xCD, 0xAA, 0xCC, 0xD2, 0xE3, 0x12};
    ble_advdata_manuf_data_t   manuf_data;

    // Setting up base UUID as manufacturing data
    manuf_data.company_identifier = COMPANY_IDENTIFER;                                  /* Company Identifier */
    manuf_data.data.size          = 16;
    manuf_data.data.p_data        = Vendor_Spec_Uuid;

    ble_uuid_t adv_uuids[] = 
    {
        {SENTRY_PROFILE_MOVEMENT_SERVICE_UUID,	  				BLE_UUID_TYPE_BLE}, 
        {SENTRY_PROFILE_PIR_SERVICE_UUID,									BLE_UUID_TYPE_BLE},
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
    m_adv_params.p_peer_addr = NULL;                                        /* Undirected advertisement */
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;
}


/**@brief Function for initializing services that will be used by the application.
*
* @details Initialize the movement/PIR alarm ,and Device Information services.
*/
static void services_init(void)
{

    movement_init();	   /* Initialize accelerometer/movement alarm service */
    pir_init();          /* Initialize passive infrared alarm service */
    dlogs_init();				 /* Initialize the data logger service */	
    device_init();       /* Initialize device management service */
    dis_init();          /* Initialize device information service */
    bas_init();	         /* Initialize battery service*/
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


/**@brief Function for initializing PIR alarm service*/
static void pir_init(void)
{

    uint32_t         err_code;
    ble_pir_init_t    pir_init;

    // Initialize PIR presence alarm Service
    memset(&pir_init, 0, sizeof(pir_init));

    // Here the sec level for the PIR Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&pir_init.pir_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&pir_init.pir_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&pir_init.pir_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&pir_init.pir_char_attr_md2.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&pir_init.battery_level_report_read_perm);

    pir_init.evt_handler          = NULL;
    pir_init.write_evt_handler    = NULL;
    pir_init.support_notification = true;
    pir_init.p_report_ref         = NULL; 
    pir_init.pir_presence_alarm   = RESET_ALARM;
    pir_init.pir_alarm_set        = DEFAULT_ALARM_SET;

    err_code = ble_pir_init(&m_pir, &pir_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing Movement alarm service*/
static void movement_init(void)
{

    uint32_t         err_code;
    ble_movement_init_t    movement_init;

    // Initialize Movement alarm Service
    memset(&movement_init, 0, sizeof(movement_init));

    // Here the sec level for the Movement Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&movement_init.movement_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&movement_init.movement_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&movement_init.movement_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&movement_init.movement_char_attr_md2.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&movement_init.battery_level_report_read_perm);

    movement_init.evt_handler          = NULL;
    movement_init.write_evt_handler    = NULL;
    movement_init.support_notification = true;
    movement_init.p_report_ref         = NULL; 
    movement_init.movement_alarm_set	 = DEFAULT_ALARM_SET; 
    movement_init.movement_alarm_clear = RESET_ALARM;
    movement_init.movement_alarm       = RESET_ALARM;

    err_code = ble_movement_init(&m_movement, &movement_init);
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

/**@brief Function for initializing data logger service*/
static void dlogs_init(void)
{

    uint32_t           err_code;
    ble_dlogs_init_t   dlogs_init;

    // Initialize 
    memset(&dlogs_init, 0, sizeof(dlogs_init));

    // Here the sec level for the data logger Service can be changed/increased.
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

    err_code = ble_dlogs_init(&m_dlogs, &dlogs_init);
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
    device_init.write_evt_handler   = NULL;
    device_init.support_notification = true;
    device_init.p_report_ref         = NULL; 

    // Set the default low value for DFU and Switch mode characteristics 
    device_init.device_dfu_mode_set          = DEFAULT_ALARM_SET;
    device_init.device_mode_switch_set       = DEFAULT_ALARM_SET;

    // Start Time satmp from initial values
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


/**@brief Function for initializing security parameters.*/
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

/**@brief Function for starting advertising.*/
static void advertising_start(void)
{
    uint32_t err_code;
    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);

    nrf_gpio_pin_set(ADVERTISING_LED_PIN_NO);
}

/**@brief Function for handling the Connection Parameters Module.
*
* @details This function will be called for all events in the Connection Parameters Module which
*          are passed to the application.
* @note All this function does is to disconnect. This could have been done by simply
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
    case BLE_EVT_TX_COMPLETE:											/*Event indicated the transmission of a characteristics is complete*/
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

    ble_dlogs_on_ble_evt(&m_dlogs, p_ble_evt); 
    ble_pir_on_ble_evt(&m_pir, p_ble_evt);
    ble_bas_on_ble_evt(&bas, p_ble_evt);
    ble_movement_on_ble_evt(&m_movement, p_ble_evt);
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
        break;

    default:
        APP_ERROR_HANDLER(pin_no);
    }
}


/**@brief event handler for  the PIR GPIOTE module.
*/
static void pir_gpiote_evt_handler(uint32_t pins_low_to_high_mask, uint32_t pins_high_to_low_mask)
{ 
    PIR_EVENT_FLAG=true;																					/*the flag is set when an event occurs on gpiote*/
}

/**@brief event handler for  the Movement GPIOTE module.
*/
static void movement_gpiote_evt_handler(uint32_t pins_low_to_high_mask, uint32_t pins_high_to_low_mask)
{
    movement_gpio_pin_val = nrf_gpio_pin_read(MOVEMENT_GPIOTE_PIN);	
    MOVEMENT_EVENT_FLAG=true;																			/*the flag is set when an event occurs on gpiote*/
}


/**@brief Function for initializing the GPIOTE handler module.
*/
static void gpiote_init(void)
{  
    uint32_t err_code;	

    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);

    // Configure GPIO pin P0.02 as input which is connected PIR sensor output
    nrf_gpio_cfg_input(PIR_GPIOTE_PIN, NRF_GPIO_PIN_PULLUP); 
    // Configure GPIO pin P0.04 as input which is connected INT1 pin of MMA7660 accelerometer
    nrf_gpio_cfg_input(MOVEMENT_GPIOTE_PIN, NRF_GPIO_PIN_PULLUP);

    // Calls an event handler whenever a HIGH->LOW or LOW->HIGH transition is incurred on P0.02 GPIO pin
    err_code = app_gpiote_user_register(&pir_measurement_gpiote, 
    PIR_PINS_LOW_TO_HIGH_MASK, 
    PIR_PINS_HIGH_TO_LOW_MASK, 
    pir_gpiote_evt_handler);        /* Register the gpiote user for PIR sensor*/
    if (err_code != NRF_SUCCESS )                                     
    {                                                                 
        APP_ERROR_HANDLER(err_code);                                   
    }                                                                 
    
    err_code = app_gpiote_user_enable(pir_measurement_gpiote);         /* Enable the registered user */
    if (err_code != NRF_SUCCESS )                                     
    {                                                                 
        APP_ERROR_HANDLER(err_code);                                    
    }                                                                 


    // Calls an event handler whenever a HIGH->LOW or LOW->HIGH transition is incurred on P0.04 GPIO pin
    err_code = app_gpiote_user_register(&movement_measurement_gpiote, 
    NULL, 
    MOVEMENT_PINS_HIGH_TO_LOW_MASK, 
    movement_gpiote_evt_handler);  /* Register the gpiote user for accelerometer */
    if (err_code != NRF_SUCCESS )                                     
    {                                                                 
        APP_ERROR_HANDLER(err_code);                                   
    }                                                                 

    err_code = app_gpiote_user_enable(movement_measurement_gpiote);    /* Enable the registered user */
    if (err_code != NRF_SUCCESS )
    {
        APP_ERROR_HANDLER(err_code);
    }

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
    ble_flash_on_radio_active_evt(m_radio_event);   /* Call the event handler*/
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


static void create_log_data(uint32_t * data)
{
    uint16_t current_pir_presence = 0;


    current_pir_presence = nrf_gpio_pin_read(PIR_GPIOTE_PIN);					

    data[0] = (m_time_stamp.year<<16)|(m_time_stamp.month<<8)|m_time_stamp.day;				 /* First word written to memory contains date (YYYYMMDD)*/
    data[1] = (m_time_stamp.hours<<16)|(m_time_stamp.minutes<<8)|m_time_stamp.seconds; /* Second word contains time HHMMSS*/
    // first word is X data,second word is Y data and third word is Z data 
    data[2] = (current_xyz_array[0] << 16) | (current_xyz_array[1] << 8) | current_xyz_array[2] ;			
    data[3] = current_pir_presence;                                                    /* Fourth word contains PIR state */

}

/**@brief Function for checking whether to log data.
*/
static void data_log_check()
{
    uint32_t log_data[4];                /*array storing the data to be logged */

    if(ENABLE_DATA_LOG && !READ_DATA)									    /*if enabled, start data logging functionality*/
    {   
        create_log_data(log_data);                        /*create the data to be logged */
        write_data_flash(log_data);	                      /*log the data to flash */
    }
}
/**@brief Function for application main entry.
*/
void connectable_mode(void)
{
    uint32_t err_code;

    // Initialization.
    ble_stack_init();
    twi_master_init(); 

    MMA7660_config_standby_and_initialize();

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
    MMA7660_enable_active_mode();
    twi_turn_OFF();
    application_timers_start();   
    advertising_start();

    // Enter main loop.
    for (;;)
    {

        // If the broadcast mode flag is true and services are not connected stop advertising and exit
        if((BROADCAST_MODE) && (!PIR_CONNECTED_STATE) && (!ACCELEROMETER_CONNECTED_STATE)) 
        {
            sd_ble_gap_adv_stop();							  /* Stop advertising */
            break;
        }

        // If the dfu enable flag is true and services are not connected go to the bootloader 
        if(DFU_ENABLE && (!DEVICE_CONNECTED_STATE) && (!PIR_CONNECTED_STATE) && (!ACCELEROMETER_CONNECTED_STATE))  
        {
            // If DFU mode is enabled , set the value of general purpose retention register to 1 
            sd_power_gpregret_set(1);            /* If DFU mode is enabled, set the general purpose retention register to 1*/
            sd_nvic_SystemReset();               /* Apply a system reset for jumping into bootloader*/                     
        }                                       

        // If the PIR event mode flag is true and go for alarm check condition
        if(PIR_EVENT_FLAG)
        {

            // Check whether alarm has to be send to indicate pir presence 
            err_code = ble_pir_alarm_check(&m_pir);             
            if ((err_code != NRF_SUCCESS) &&
                    (err_code != NRF_ERROR_INVALID_STATE) &&
                    (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
                    (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                    )
            {
                APP_ERROR_HANDLER(err_code);
            }  

            PIR_EVENT_FLAG=false;				         /* Reset the gpiote event flag*/

        }

        // If the Movement event mode flag is true and go for alarm check condition			 
        if(MOVEMENT_EVENT_FLAG)
        {
            // Check whether alarm has to be send to indicate movement 
            err_code = ble_movement_alarm_check(&m_movement);            
            if ((err_code != NRF_SUCCESS) &&
                    (err_code != NRF_ERROR_INVALID_STATE) &&
                    (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
                    (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                    )
            {
                APP_ERROR_HANDLER(err_code);
            }  

            MOVEMENT_EVENT_FLAG=false;					 /* Reset the gpiote event flag*/
        }
        if (DATA_LOG_CHECK)
        {
            data_log_check();
            DATA_LOG_CHECK= false;
        }


        // If READ_DATA flag is set, start sending data to the connected device	
        if(READ_DATA)																			                /* If enabled, start sending data to the connected device */
        {
            err_code=app_timer_stop(sentry_measurement_timer);		        /* Stop the timers before start sending the historical data */
            APP_ERROR_CHECK(err_code);
            err_code=app_gpiote_user_disable(pir_measurement_gpiote);     /* Disable the PIR gpiote*/
            err_code=app_gpiote_user_disable(movement_measurement_gpiote);/* Disable the movement gpiote*/
            APP_ERROR_CHECK(err_code);
            READ_DATA = false;
            ENABLE_DATA_LOG = false;                                      /* Disable data logging functionality */
            send_data(&m_dlogs);																          /* Start sending the data */												
            application_timers_start();												            /* Restart the timers when sending is finished */
            err_code=app_gpiote_user_enable(pir_measurement_gpiote);      /* Re-enable PIR gpiote */
            err_code=app_gpiote_user_enable(movement_measurement_gpiote); /* Re-enable movement gpiote */
            APP_ERROR_CHECK(err_code);
            err_code=reset_data_log(&m_dlogs);									          /* Reset the data logger enable and data read switches*/
            APP_ERROR_CHECK(err_code);	
        }    

        // If TIME_SET flag is set, create new time stamp
        if(TIME_SET)
        {
            create_time_stamp(&m_device, &m_time_stamp);          /* Create new time stamp from user set time*/
            TIME_SET = false;                                              /* Reset the flag*/
            
        }                                                                 

        if (CLEAR_MOVE_ALARM)	
        {   
            err_code = reset_alarm(&m_movement);             /* Check whether alarm clear in movement service is set */
            if ((err_code != NRF_SUCCESS) &&
                    (err_code != NRF_ERROR_INVALID_STATE) &&
                    (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
                    (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                    )
            {
                APP_ERROR_HANDLER(err_code);

            } 
            CLEAR_MOVE_ALARM= false;
        }				 
        
        power_manage(); 

    }

}


// * @}
