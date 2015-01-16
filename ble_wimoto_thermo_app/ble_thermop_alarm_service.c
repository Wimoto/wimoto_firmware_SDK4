/** @file
*
* @{
* @ingroup ble_sdk_app_hts
* @brief Thermopile temperature Service file.
*
* This file contains the source code for Thermopile temperature alarm service
* It also includes the code for Device Information services.
* This application uses the @ref srvlib_conn_params module.
* Adapted from  Battery service provided by Nordic
* 
* Author : Sherin 
* Date   : 11/21/2013
* 
* Change log
* Sherin           12/10/2013     Added write events for value fields
* Hariprasad       12/11/2013     Added 128bit Vendor specific  custom UUID's for the service and all characteristics 
*/

#include "ble_thermop_alarm_service.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "wimoto.h"
#include "wimoto_sensors.h"

bool     	      THERMOPS_CONNECTED_STATE=false;  /*Indicates whether the thermopile service is connected or not*/
extern bool 	  CHECK_ALARM_TIMEOUT;
uint8_t         current_thermopile_temp_store[THERMOP_CHAR_SIZE];

/**@brief Function for handling the Connect event.
*
* @param[in]   p_thermops       Thermopile Service structure.
* @param[in]   p_ble_evt   Event received from the BLE stack.
*/
static void on_connect(ble_thermops_t * p_thermops, ble_evt_t * p_ble_evt)
{
    p_thermops->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    THERMOPS_CONNECTED_STATE = true;  /*Set the flag to true so that state remains in connectable mode until disconnect*/
}


/**@brief Function for handling the Disconnect event.
*
* @param[in]   p_thermops       Thermopile Service structure.
* @param[in]   p_ble_evt   Event received from the BLE stack.
*/
static void on_disconnect(ble_thermops_t * p_thermops, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    THERMOPS_CONNECTED_STATE= false; 
    p_thermops->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**@brief Function for handling write events on values.
*
* @details This function will be called for all write events of temperature low, high values and alarm set 
*          which are passed to the application.
*/
static void write_evt_handler(void)
{   
    CHECK_ALARM_TIMEOUT = true; 
}
/**@brief Function for handling the Write event.
*
* @param[in]   p_temps       Temperature Service structure.
* @param[in]   p_ble_evt   Event received from the BLE stack.
*/

/**@brief Function for handling the Write event.
*
* @param[in]   p_thermops       Thermopile Service structure.
* @param[in]   p_ble_evt   Event received from the BLE stack.
*/
static void on_write(ble_thermops_t * p_thermops, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_thermops->is_notification_supported)
    {

        //write event for thermopile low value char cccd.   
        if (
                (p_evt_write->handle == p_thermops->thermop_low_level_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_thermops->evt_handler != NULL)
            {
                ble_thermops_low_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_DISABLED;
                }

                p_thermops->evt_handler(p_thermops, &evt);
            }
        }

        //write event for thermopile high value char cccd. 

        if (
                (p_evt_write->handle == p_thermops->thermop_high_level_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_thermops->evt_handler != NULL)
            {
                ble_thermops_low_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_DISABLED;
                }

                p_thermops->evt_handler(p_thermops, &evt);
            }
        }

        //write event for thermopile alarm set cccd

        if (
                (p_evt_write->handle == p_thermops->thermo_thermop_alarm_set_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_thermops->evt_handler != NULL)
            {
                ble_thermops_low_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_DISABLED;
                }

                p_thermops->evt_handler(p_thermops, &evt);
            }
        } 

        //write event for thermopile alarm cccd

        if (
                (p_evt_write->handle == p_thermops->thermo_thermop_alarm_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_thermops->evt_handler != NULL)
            {
                ble_thermops_low_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_DISABLED;
                }

                p_thermops->evt_handler(p_thermops, &evt);
            }
        } 

        //write event for thermopile cccd
        if (
                (p_evt_write->handle == p_thermops->current_thermop_handles.cccd_handle)
                &&
                (p_evt_write->len == 2)
                )
        {
            // CCCD written, call application event handler
            if (p_thermops->evt_handler != NULL)
            {
                ble_thermops_low_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_THERMOPS_LOW_EVT_NOTIFICATION_DISABLED;
                }

                p_thermops->evt_handler(p_thermops, &evt);
            }
        } 



    }

    // write event for temperature low value char value. 
    if (
            (p_evt_write->handle == p_thermops->thermop_low_level_handles.value_handle) 
            && 
            (p_evt_write->len == 5)
            )
    {
        // update the temperature service structure
        p_thermops->thermo_thermopile_low_level[0] =   p_evt_write->data[0];
        p_thermops->thermo_thermopile_low_level[1] =   p_evt_write->data[1];
        p_thermops->thermo_thermopile_low_level[2] =   p_evt_write->data[2];
        p_thermops->thermo_thermopile_low_level[3] =   p_evt_write->data[3];
        p_thermops->thermo_thermopile_low_level[4] =   p_evt_write->data[4];

        // call application event handler
        p_thermops->write_evt_handler();
    }

    // write event for temperature high value char value. 
    if (
            (p_evt_write->handle == p_thermops->thermop_high_level_handles.value_handle) 
            && 
            (p_evt_write->len == 5)
            )
    {
        // update the temperature service structure
        p_thermops->thermo_thermopile_high_level[0] =   p_evt_write->data[0];
        p_thermops->thermo_thermopile_high_level[1] =   p_evt_write->data[1];
        p_thermops->thermo_thermopile_high_level[2] =   p_evt_write->data[2];
        p_thermops->thermo_thermopile_high_level[3] =   p_evt_write->data[3];
        p_thermops->thermo_thermopile_high_level[4] =   p_evt_write->data[4];

        // call application event handler
        p_thermops->write_evt_handler();
    }

    // write event for alarm set value char value. 
    if (
            (p_evt_write->handle == p_thermops->thermo_thermop_alarm_set_handles.value_handle) 
            && 
            (p_evt_write->len == 1)
            )
    {
        // update the temperature service structure
        p_thermops->thermo_thermopile_alarm_set =   p_evt_write->data[0];

        // call application event handler
        p_thermops->write_evt_handler();
    }		
}


void ble_thermops_on_ble_evt(ble_thermops_t * p_thermops, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        on_connect(p_thermops, p_ble_evt);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        on_disconnect(p_thermops, p_ble_evt);
        break;

    case BLE_GATTS_EVT_WRITE:
        on_write(p_thermops, p_ble_evt);
        break;

    default:
        break;
    }

}


/**@brief Function for adding the current thermopile characteristics.
*
* @param[in]   p_thermops        Thermopile Service structure.
* @param[in]   p_thermops_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/
static uint32_t current_thermopile_char_add(ble_thermops_t * p_thermops, const ble_thermops_init_t * p_thermops_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      current_thermopile[5];

    if (p_thermops->is_notification_supported)
    {
        memset(&cccd_md, 0, sizeof(cccd_md));
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        cccd_md.write_perm = p_thermops_init->thermopile_char_attr_md.cccd_write_perm;
        cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read         = 1;
    char_md.char_props.notify       = (p_thermops->is_notification_supported) ? 1 : 0;
    char_md.p_char_pf               = NULL;
    char_md.p_user_desc_md          = NULL;
    char_md.p_cccd_md               = (p_thermops->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md               = NULL;

    // Adding custom UUID
    ble_uuid.type = p_thermops->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_THERMOPS_CURR_THERMOP_UUID;    

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_thermops_init->thermopile_char_attr_md.read_perm;
    attr_md.write_perm = p_thermops_init->thermopile_char_attr_md2.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(current_thermopile);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(current_thermopile);
    attr_char_value.p_value      = current_thermopile;

    err_code = sd_ble_gatts_characteristic_add(p_thermops->service_handle, &char_md,
    &attr_char_value,
    &p_thermops->current_thermop_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}



/**@brief Function for adding the thermopile low value characteristics.
*
* @param[in]   p_thermops        Thermopile Service structure.
* @param[in]   p_thermops_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/
static uint32_t thermopile_low_value_char_add(ble_thermops_t * p_thermops, const ble_thermops_init_t * p_thermops_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      thermo_thermopile_low_level[5];

    // Add Thermopile low value characteristic
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.notify   = 0;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = NULL;
    char_md.p_sccd_md           = NULL;

    // Adding custom UUID
    ble_uuid.type = p_thermops->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_THERMOPS_LOW_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_thermops_init->thermopile_char_attr_md.read_perm;
    attr_md.write_perm = p_thermops_init->thermopile_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    thermo_thermopile_low_level[4] = p_thermops_init->thermo_thermopile_low_level[4];
    thermo_thermopile_low_level[3] = p_thermops_init->thermo_thermopile_low_level[3];
    thermo_thermopile_low_level[2] = p_thermops_init->thermo_thermopile_low_level[2];		
    thermo_thermopile_low_level[1] = p_thermops_init->thermo_thermopile_low_level[1];
    thermo_thermopile_low_level[0] = p_thermops_init->thermo_thermopile_low_level[0];		
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(thermo_thermopile_low_level);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(thermo_thermopile_low_level);
    attr_char_value.p_value      = thermo_thermopile_low_level;

    err_code = sd_ble_gatts_characteristic_add(p_thermops->service_handle, &char_md,
    &attr_char_value,
    &p_thermops->thermop_low_level_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the thermopile high value characteristics.
*
* @param[in]   p_thermops        Thermopile Service structure.
* @param[in]   p_thermops_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/

static uint32_t thermopile_high_value_char_add(ble_thermops_t * p_thermops, const ble_thermops_init_t * p_thermops_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      thermo_thermopile_high_level[5];


    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.notify   = 0;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = NULL;
    char_md.p_sccd_md           = NULL;

    // Adding custom UUID
    ble_uuid.type = p_thermops->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_THERMOPS_HIGH_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_thermops_init->thermopile_char_attr_md.read_perm;
    attr_md.write_perm = p_thermops_init->thermopile_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    thermo_thermopile_high_level[4] = p_thermops_init->thermo_thermopile_high_level[4];
    thermo_thermopile_high_level[3] = p_thermops_init->thermo_thermopile_high_level[3];  /*Default values for thermopile high value*/
    thermo_thermopile_high_level[2] = p_thermops_init->thermo_thermopile_high_level[2];	
    thermo_thermopile_high_level[1] = p_thermops_init->thermo_thermopile_high_level[1];  /*Default values for thermopile high value*/
    thermo_thermopile_high_level[0] = p_thermops_init->thermo_thermopile_high_level[0];	

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(thermo_thermopile_high_level);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(thermo_thermopile_high_level);
    attr_char_value.p_value      = thermo_thermopile_high_level;

    err_code = sd_ble_gatts_characteristic_add(p_thermops->service_handle, &char_md,
    &attr_char_value,
    &p_thermops->thermop_high_level_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the thermopile alarm set characteristics.
*
* @param[in]   p_thermops        Thermopile Service structure.
* @param[in]   p_thermops_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/

static uint32_t thermopile_alarm_set_char_add(ble_thermops_t * p_thermops, const ble_thermops_init_t * p_thermops_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t      thermo_thermopile_alarm_set;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.notify   = 0;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = NULL;
    char_md.p_sccd_md           = NULL;

    // Adding custom UUID
    ble_uuid.type = p_thermops->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_THERMOPS_ALARM_SET_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_thermops_init->thermopile_char_attr_md.read_perm;
    attr_md.write_perm = p_thermops_init->thermopile_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    thermo_thermopile_alarm_set = p_thermops_init->thermo_thermopile_alarm_set;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = &thermo_thermopile_alarm_set;

    err_code = sd_ble_gatts_characteristic_add(p_thermops->service_handle, &char_md,
    &attr_char_value,
    &p_thermops->thermo_thermop_alarm_set_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the thermopile alarm characteristics.
*
* @param[in]   p_thermops        Thermopile Service structure.
* @param[in]   p_thermops_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/

static uint32_t thermopile_alarm_char_add(ble_thermops_t * p_thermops, const ble_thermops_init_t * p_thermops_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    static uint8_t             thermo_thermopile_alarm;

    // Add thermopile high level characteristic 
    if (p_thermops->is_notification_supported)
    {
        memset(&cccd_md, 0, sizeof(cccd_md));

        // According to BAS_SPEC_V10, the read operation on cccd should be possible without
        // authentication.
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        cccd_md.write_perm = p_thermops_init->thermopile_char_attr_md.cccd_write_perm;
        cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = (p_thermops->is_notification_supported) ? 1 : 0;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = (p_thermops->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

    // Adding custom UUID
    ble_uuid.type = p_thermops->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_THERMOPS_ALARM_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_thermops_init->thermopile_char_attr_md.read_perm;
    attr_md.write_perm = p_thermops_init->thermopile_char_attr_md2.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;


    thermo_thermopile_alarm = p_thermops_init->thermo_thermopile_alarm;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = &thermo_thermopile_alarm;

    err_code = sd_ble_gatts_characteristic_add(p_thermops->service_handle, &char_md,
    &attr_char_value,
    &p_thermops->thermo_thermop_alarm_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


/**@brief Function for initializing the thermopile.
*
* @param[in]   p_thermops        Thermopile Service structure.
* @param[in]   p_thermops_init   Information needed to initialize the service.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/

uint32_t ble_thermops_init(ble_thermops_t * p_thermops, const ble_thermops_init_t * p_thermops_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;
    ble_uuid128_t base_uuid = THERMO_PROFILE_BASE_UUID;

    // Add custom base UUID
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_thermops->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add service
    ble_uuid.type = p_thermops->uuid_type;
    ble_uuid.uuid = THERMO_PROFILE_SERVICE_UUID;

    // Initialize service structure
    p_thermops->evt_handler               = p_thermops_init->evt_handler;
    p_thermops->write_evt_handler         = write_evt_handler;
    p_thermops->conn_handle               = BLE_CONN_HANDLE_INVALID;
    p_thermops->is_notification_supported = p_thermops_init->support_notification;


    p_thermops->thermo_thermopile_high_level[4] = p_thermops_init->thermo_thermopile_high_level[4];
    p_thermops->thermo_thermopile_high_level[3] = p_thermops_init->thermo_thermopile_high_level[3];  /*Default values for thermopile high value*/
    p_thermops->thermo_thermopile_high_level[2] = p_thermops_init->thermo_thermopile_high_level[2];	
    p_thermops->thermo_thermopile_high_level[1] = p_thermops_init->thermo_thermopile_high_level[1];  /*Default values for thermopile high value*/
    p_thermops->thermo_thermopile_high_level[0] = p_thermops_init->thermo_thermopile_high_level[0];	

    p_thermops->thermo_thermopile_low_level[4] = p_thermops_init->thermo_thermopile_low_level[4];
    p_thermops->thermo_thermopile_low_level[3] = p_thermops_init->thermo_thermopile_low_level[3];
    p_thermops->thermo_thermopile_low_level[2] = p_thermops_init->thermo_thermopile_low_level[2];		
    p_thermops->thermo_thermopile_low_level[1] = p_thermops_init->thermo_thermopile_low_level[1];
    p_thermops->thermo_thermopile_low_level[0] = p_thermops_init->thermo_thermopile_low_level[0];		

    p_thermops->thermo_thermopile_alarm_set    = p_thermops_init->thermo_thermopile_alarm_set;
    p_thermops->thermo_thermopile_alarm        = p_thermops_init->thermo_thermopile_alarm;



    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_thermops->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }



    err_code =  current_thermopile_char_add(p_thermops, p_thermops_init);   /* Add current thermopile characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code =  thermopile_low_value_char_add(p_thermops, p_thermops_init); /* Add thermopile low value characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code =  thermopile_high_value_char_add(p_thermops, p_thermops_init); /* Add thermopile high value characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code =  thermopile_alarm_set_char_add(p_thermops, p_thermops_init); /* Add thermopile alarm set characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code =  thermopile_alarm_char_add(p_thermops, p_thermops_init);     /* Add thermopile alarm characteristic*/
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;

}

/**@brief Function reads and updates the current thermopile and checks for alarm condition.
*
* @param[in]   p_thermops        Thermopile Service structure.
*
* @return      NRF_SUCCESS on success, otherwise an error code.
*/
uint32_t ble_thermops_level_alarm_check(ble_thermops_t * p_thermops)
{
    uint32_t err_code = NRF_SUCCESS;
    float current_thermopile;
    uint8_t  current_thermopile_array[THERMOP_CHAR_SIZE];
    float thermopile_low_value;		
    float thermopile_high_value;		

    static float  previous_thermopile = 0x00;
    uint8_t alarm = 0x00;

    uint16_t len = sizeof(uint8_t);
    uint16_t len1 = sizeof(current_thermopile_array);

    read_thermopile_connectable(current_thermopile_array, &current_thermopile); /* read the current thermopile*/

    int i;
    for (i=0;i<THERMOP_CHAR_SIZE;i++)
    {
        current_thermopile_temp_store[i] = current_thermopile_array[i]; /*store the current thermopile to a global array to be used for data logging*/
    }

    if(current_thermopile != previous_thermopile)                   /*Check whether thermopile value has changed*/
    {   
        previous_thermopile         = current_thermopile;


        if ((p_thermops->conn_handle != BLE_CONN_HANDLE_INVALID) && p_thermops->is_notification_supported)
        {
            ble_gatts_hvx_params_t hvx_params;

            memset(&hvx_params, 0, sizeof(hvx_params));


            hvx_params.handle   = p_thermops->current_thermop_handles.value_handle;
            hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset   = 0;
            hvx_params.p_len    = &len1;
            hvx_params.p_data   = current_thermopile_array;

            err_code = sd_ble_gatts_hvx(p_thermops->conn_handle, &hvx_params);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
    }		

    /*Get the thermopile low value set by the user from the service */
    thermopile_low_value  = stof((char *)(p_thermops->thermo_thermopile_low_level));

    /*Get the thermopile high value set by the user from the service */
    thermopile_high_value = stof(((char *)p_thermops->thermo_thermopile_high_level));



    /*Check whether the thermopile is out of range if alarm is set by user */		
    if(p_thermops->thermo_thermopile_alarm_set != 0x00)
    {


        if(current_thermopile < thermopile_low_value)
        {
            alarm = SET_ALARM_THERMOP_LOW;		//set alarm to 01 if thermopile is low 
        }

        else if(current_thermopile > thermopile_high_value)
        {
            alarm = SET_ALARM_THERMOP_HIGH;		//set alarm to 02 if thermopile is high 
        } 

        else
        {	
            alarm = RESET_ALARM;		       //reset alarm to 0x00
        }	
    }
    else
    {	
        alarm = RESET_ALARM;								           /*reset alarm to 0x00*/
    }		


    if(alarm != p_thermops->thermo_thermopile_alarm )  /*check whether the alarm value has changed and send the change*/
    {	
        p_thermops->thermo_thermopile_alarm = alarm;

        // Send value if connected and notifying


        if ((p_thermops->conn_handle != BLE_CONN_HANDLE_INVALID) && p_thermops->is_notification_supported)
        {
            ble_gatts_hvx_params_t hvx_params;

            memset(&hvx_params, 0, sizeof(hvx_params));


            hvx_params.handle   = p_thermops->thermo_thermop_alarm_handles.value_handle;
            hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset   = 0;
            hvx_params.p_len    = &len;
            hvx_params.p_data   = &alarm;

            err_code = sd_ble_gatts_hvx(p_thermops->conn_handle, &hvx_params);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
    }


    return err_code;

}

/**@brief Function to read thermopile from tmp006.
*
* @param[out]   uint8_t *buffer 
* @param[out]   float *fTemp
*/
void read_thermopile_connectable(uint8_t *buffer, float *fTemp)   
{
    twi_turn_ON();
    do_thermopile_measurement(buffer, fTemp);
    twi_turn_OFF();
    return;
}	

