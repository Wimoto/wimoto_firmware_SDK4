/** @file
*
* @{
* @brief probe temperature Service file.
*
* This file contains the source code for probe temperature level alarm service
* Adapted from  Battery service provided by Nordic
* 
* Author : Sherin Deena
* Date   : 11/21/2013
* Change log
* Sherin           12/10/2013     Added write events for value fields
* Hariprasad       12/11/2013     Added 128bit Vendor specific  custom UUID's for the service and all characteristics 
*/

#include "ble_probe_alarm_service.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "wimoto.h"
#include "wimoto_sensors.h"

bool PROBES_CONNECTED_STATE=false;      /*This flag indicates whether a client is connected to the peripheral in probe temperature service*/
extern bool    BROADCAST_MODE;
extern bool 	 CHECK_ALARM_TIMEOUT;

/**@brief Function for handling the Connect event.
*
* @param[in]   p_probes    probe temperature Service structure.
* @param[in]   p_ble_evt   Event received from the BLE stack.
*/
static void on_connect(ble_probes_t * p_probes, ble_evt_t * p_ble_evt)
{
    p_probes->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    PROBES_CONNECTED_STATE = true;      /*Set the flag to true so that state remains in connectable mode until disconnected*/
}


/**@brief Function for handling the Disconnect event.
*
* @param[in]   p_probes    probe temperature Service structure.
* @param[in]   p_ble_evt   Event received from the BLE stack.
*/
static void on_disconnect(ble_probes_t * p_probes, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    PROBES_CONNECTED_STATE = false;     /*Indicate that probe temperature service is disconnected*/
    p_probes->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**@brief Function for handling write events on values.
*
* @details This function will be called for all write events of probe temperature low, high values and alarm set 
*          which are passed to the application.
*/
static void write_evt_handler(void)
{   
    CHECK_ALARM_TIMEOUT = true; 
}

/**@brief Function for handling the Write event.
*
* @param[in]   p_probes      probe temperature Service structure.
* @param[in]   p_ble_evt   Event received from the BLE stack.
*/
static void on_write(ble_probes_t * p_probes, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_probes->is_notification_supported)
    {

        //write event for probe temperature low value char.   
        if (
                (p_evt_write->handle == p_probes->probe_temp_low_level_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_probes->evt_handler != NULL)
            {
                ble_probes_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_DISABLED;
                }

                p_probes->evt_handler(p_probes, &evt);
            }
        }

        //write event for probe temperature level high value char. 

        if (
                (p_evt_write->handle == p_probes->probe_temp_high_level_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_probes->evt_handler != NULL)
            {
                ble_probes_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_DISABLED;
                }

                p_probes->evt_handler(p_probes, &evt);
            }
        }

        //write event for probe temperature alarm set

        if (
                (p_evt_write->handle == p_probes->probe_temp_alarm_set_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_probes->evt_handler != NULL)
            {
                ble_probes_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_DISABLED;
                }

                p_probes->evt_handler(p_probes, &evt);
            }
        } 

        //write event for probe temperature alarm

        if (
                (p_evt_write->handle == p_probes->probe_temp_alarm_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_probes->evt_handler != NULL)
            {
                ble_probes_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_DISABLED;
                }

                p_probes->evt_handler(p_probes, &evt);
            }
        } 

        //write event for current probe temperature level


        if (
                (p_evt_write->handle == p_probes->curr_probe_temp_level_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_probes->evt_handler != NULL)
            {
                ble_probes_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_PROBES_EVT_NOTIFICATION_DISABLED;
                }

                p_probes->evt_handler(p_probes, &evt);
            }
        }
    }

    // write event for soil moisture low value char value. 
    if (
            (p_evt_write->handle == p_probes->probe_temp_low_level_handles.value_handle) 
            && 
            (p_evt_write->len == 1)
            )
    {
        // update the temperature service structure
        p_probes->probe_temp_low_level =   p_evt_write->data[0];

        // call application event handler
        p_probes->write_evt_handler();
    }		

    // write event for soil moisture high value char value. 
    if (
            (p_evt_write->handle == p_probes->probe_temp_high_level_handles.value_handle) 
            && 
            (p_evt_write->len == 1)
            )
    {
        // update the temperature service structure
        p_probes->probe_temp_high_level =   p_evt_write->data[0];

        // call application event handler
        p_probes->write_evt_handler();
    }

    // write event for alarm set value char value. 
    if (
            (p_evt_write->handle == p_probes->probe_temp_alarm_set_handles.value_handle) 
            && 
            (p_evt_write->len == 1)
            )
    {
        // update the temperature service structure
        p_probes->probe_temp_alarm_set =   p_evt_write->data[0];

        // call application event handler
        p_probes->write_evt_handler();
    }		

}


void ble_probes_on_ble_evt(ble_probes_t * p_probes, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        on_connect(p_probes, p_ble_evt);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        on_disconnect(p_probes, p_ble_evt);
        break;

    case BLE_GATTS_EVT_WRITE:
        on_write(p_probes, p_ble_evt);
        break;

    default:
        break;
    }

}

/**@brief Function for adding the current probe temperature level characteristics.
*
* @param[in]   p_probes        probe temperature Service structure.
* @param[in]   p_probes_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/
static uint32_t current_probe_temp_level_char_add(ble_probes_t * p_probes, const ble_probes_init_t * p_probes_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      current_probe_temp_level;

    if (p_probes->is_notification_supported)
    {
        memset(&cccd_md, 0, sizeof(cccd_md));
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        cccd_md.write_perm = p_probes_init->probe_temp_char_attr_md.cccd_write_perm;
        cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read         = 1;
    char_md.char_props.notify       = (p_probes->is_notification_supported) ? 1 : 0;
    char_md.p_char_pf               = NULL;
    char_md.p_user_desc_md          = NULL;
    char_md.p_cccd_md               = (p_probes->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md               = NULL;


    // Adding custom UUID
    ble_uuid.type = p_probes->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_CURR_PROBE_CHAR_UUID;   //using a custom characteristics uuid

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_probes_init->probe_temp_char_attr_md.read_perm;
    attr_md.write_perm = p_probes_init->probe_temp_char_attr_md2.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(current_probe_temp_level);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(current_probe_temp_level);
    attr_char_value.p_value      = &current_probe_temp_level;

    err_code = sd_ble_gatts_characteristic_add(p_probes->service_handle, &char_md,
    &attr_char_value,
    &p_probes->curr_probe_temp_level_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the probe temperature low value characteristics.
*
* @param[in]   p_probes        probe temperature Service structure.
* @param[in]   p_probes_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/
static uint32_t probe_temp_low_value_char_add(ble_probes_t * p_probes, const ble_probes_init_t * p_probes_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      probe_temp_low_value;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.notify   = 0;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = NULL;
    char_md.p_sccd_md           = NULL;

    // Adding custom UUID
    ble_uuid.type = p_probes->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_PROBE_LOW_CHAR_UUID;   //using a custom characteristics uuid

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_probes_init->probe_temp_char_attr_md.read_perm;
    attr_md.write_perm = p_probes_init->probe_temp_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    probe_temp_low_value = p_probes_init->probe_temp_low_value;


    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(probe_temp_low_value);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(probe_temp_low_value);
    attr_char_value.p_value      = &probe_temp_low_value;

    err_code = sd_ble_gatts_characteristic_add(p_probes->service_handle,&char_md,
    &attr_char_value,
    &p_probes->probe_temp_low_level_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the probe temperature high value characteristics.
*
* @param[in]   p_probes        probe temperature Service structure.
* @param[in]   p_probes_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/

static uint32_t probe_temp_high_value_char_add(ble_probes_t * p_probes, const ble_probes_init_t * p_probes_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      probe_temp_high_value;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.notify   = 0;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = NULL;
    char_md.p_sccd_md           = NULL;

    // Adding custom UUID
    ble_uuid.type = p_probes->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_PROBE_HIGH_CHAR_UUID;   //using a custom characteristics uuid

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_probes_init->probe_temp_char_attr_md.read_perm;
    attr_md.write_perm = p_probes_init->probe_temp_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    probe_temp_high_value = p_probes_init->probe_temp_high_value;  /*Default values for probe temperature high value*/

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(probe_temp_high_value);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(probe_temp_high_value);
    attr_char_value.p_value      = &probe_temp_high_value;

    err_code = sd_ble_gatts_characteristic_add(p_probes->service_handle, &char_md,
    &attr_char_value,
    &p_probes->probe_temp_high_level_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the probe temperature alarm set characteristics.
*
* @param[in]   p_probes        probe temperature Service structure.
* @param[in]   p_probes_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/

static uint32_t probe_temp_alarm_set_char_add(ble_probes_t * p_probes, const ble_probes_init_t * p_probes_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      probe_temp_alarm_set;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.notify   = 0;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = NULL;
    char_md.p_sccd_md           = NULL;

    // Adding custom UUID
    ble_uuid.type = p_probes->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_PROBE_ALARM_SET_CHAR_UUID;   //using a custom characteristics uuid

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_probes_init->probe_temp_char_attr_md.read_perm;
    attr_md.write_perm = p_probes_init->probe_temp_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    probe_temp_alarm_set = p_probes_init->probe_temp_alarm_set;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = &probe_temp_alarm_set;

    err_code = sd_ble_gatts_characteristic_add(p_probes->service_handle, &char_md,
    &attr_char_value,
    &p_probes->probe_temp_alarm_set_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the probe temperature level alarm characteristics.
*
* @param[in]   p_probes       probe temperature Service structure.
* @param[in]   p_probes_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/

static uint32_t probe_temp_alarm_char_add(ble_probes_t * p_probes, const ble_probes_init_t * p_probes_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      probe_temp_alarm; 

    probe_temp_alarm = RESET_ALARM;  /*Initialize alarm to 0x00*/

    // Add probe temperature high level characteristic 
    if (p_probes->is_notification_supported)
    {
        memset(&cccd_md, 0, sizeof(cccd_md));
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        cccd_md.write_perm = p_probes_init->probe_temp_char_attr_md.cccd_write_perm;
        cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = (p_probes->is_notification_supported) ? 1 : 0;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = (p_probes->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

    // Adding custom UUID
    ble_uuid.type = p_probes->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_PROBE_ALARM_CHAR_UUID;   //using a custom characteristics uuid

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_probes_init->probe_temp_char_attr_md.read_perm;
    attr_md.write_perm = p_probes_init->probe_temp_char_attr_md2.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;


    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = &probe_temp_alarm;

    err_code = sd_ble_gatts_characteristic_add(p_probes->service_handle, &char_md,
    &attr_char_value,
    &p_probes->probe_temp_alarm_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for initializing the probe temperature service.
*
* @param[in]   p_probes        probe temperature Service structure.
* @param[in]   p_probes_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/

uint32_t ble_probes_init(ble_probes_t * p_probes, const ble_probes_init_t * p_probes_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;
    ble_uuid128_t base_uuid = THERMO_PROFILE_BASE_UUID;

    // Add custom base UUID
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_probes->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add service
    ble_uuid.type = p_probes->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_PROBES_SERVICE_UUID;

    // Initialize service structure
    p_probes->evt_handler               = p_probes_init->evt_handler;
    p_probes->write_evt_handler         = write_evt_handler;
    p_probes->conn_handle               = BLE_CONN_HANDLE_INVALID;
    p_probes->is_notification_supported = p_probes_init->support_notification;

    p_probes->probe_temp_low_level      = p_probes_init->probe_temp_low_value;  
    p_probes->probe_temp_high_level     = p_probes_init->probe_temp_high_value; 
    p_probes->probe_temp_alarm_set      = p_probes_init->probe_temp_alarm_set;  
    p_probes->probe_temp_alarm   	      = p_probes_init->probe_temp_alarm;   		

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_probes->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code =  current_probe_temp_level_char_add(p_probes, p_probes_init); /* Add current probe temperature level characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code =  probe_temp_low_value_char_add(p_probes, p_probes_init);    /* Add probe temperature level low value characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code =  probe_temp_high_value_char_add(p_probes, p_probes_init);   /* Add probe temperature level high value characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code =  probe_temp_alarm_set_char_add(p_probes, p_probes_init);    /* Add probe temperature level alarm set characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code =  probe_temp_alarm_char_add(p_probes, p_probes_init);       /* Add probe temperature level alarm characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return NRF_SUCCESS;

}

/**@brief Function reads and updates the current probe temperature level and checks for alarm condition.
*
* @param[in]   p_probes        probe temperature  Service structure.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/
uint32_t ble_probes_level_alarm_check(ble_probes_t * p_probes)
{
    uint32_t err_code = NRF_SUCCESS;
    uint8_t current_probe_temp_level;

    static uint16_t previous_probe_temp_level = 0x00;
    uint8_t alarm = 0x00;


    uint16_t	len = sizeof(uint8_t);

    current_probe_temp_level = read_probe_temp_level(); /* read the current probe temperature level*/


    if(current_probe_temp_level != previous_probe_temp_level)  /*Check whether probe temperature value has changed*/
    {
        previous_probe_temp_level = current_probe_temp_level;


        // Send value if connected and notifying		
        if ((p_probes->conn_handle != BLE_CONN_HANDLE_INVALID) && p_probes->is_notification_supported)
        {
            ble_gatts_hvx_params_t hvx_params;

            memset(&hvx_params, 0, sizeof(hvx_params));
            len = sizeof(uint8_t);

            hvx_params.handle   = p_probes->curr_probe_temp_level_handles.value_handle;
            hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset   = 0;
            hvx_params.p_len    = &len;
            hvx_params.p_data   = &current_probe_temp_level;

            err_code = sd_ble_gatts_hvx(p_probes->conn_handle, &hvx_params);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
    }		



    /*Check whether the probe temperature is out of range if alarm is set by user */		
    if(p_probes->probe_temp_alarm_set  != 0x00)
    {

        if(current_probe_temp_level < p_probes->probe_temp_low_level)
        {
            alarm = SET_ALARM_LOW;		           /*set alarm to 01 if probe temperature level is low */
        }

        else if(current_probe_temp_level > p_probes->probe_temp_high_level )
        {
            alarm = SET_ALARM_HIGH;		          /*set alarm to 02 if probe temperature level is high */
        } 

        else
        {	
            alarm = RESET_ALARM;		            /*reset alarm to 0x00*/
        }	
    }
    else
    {	
        alarm = RESET_ALARM;							      /*reset alarm to 0x00*/
    }		


    if(alarm != p_probes->probe_temp_alarm   )  /*check whether the alarm value has changed and send the change*/
    {	
        p_probes->probe_temp_alarm = alarm;        

        // Send value if connected and notifying

        if ((p_probes->conn_handle != BLE_CONN_HANDLE_INVALID) && p_probes->is_notification_supported)
        {
            ble_gatts_hvx_params_t hvx_params;

            memset(&hvx_params, 0, sizeof(hvx_params));
            len = sizeof(uint8_t);

            hvx_params.handle   = p_probes->probe_temp_alarm_handles.value_handle;
            hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset   = 0;
            hvx_params.p_len    = &len;
            hvx_params.p_data   = &alarm;

            err_code = sd_ble_gatts_hvx(p_probes->conn_handle, &hvx_params);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
    }
    return err_code;
}

/**@brief Function to read probe temperature level from sensor interfaced to ADC.
*
* @param[in]   void
* @param[out]  uint16_t current_probe_temp_level.
*/
uint8_t read_probe_temp_level()   
{
    static uint8_t current_probe_temp_level=0x00;                            
    current_probe_temp_level = do_probe_temperature_measurement();  /*Read probe temperature level from ADC */
    return current_probe_temp_level;
}	







