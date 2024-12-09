/*
 * @file sim800l_sms.c
 * @author Eduardo Gomes
 * @brief Source file for SIM800L SMS functions
 * 
 * @copyright MIT
 * 
 */

/*
 *     Includes
 */
#include "sim800l_core.h"
#include "sim800l_sms.h"
#include <esp_log.h>

/*
 *     Tag
 */
#define SIM800L_SMS_TAG "SIM800L SMS"

/*
 *     Public functions development
 */
esp_err_t sim800l_sms_sel_mode(sim800l_handle_t sim800l_handle, sim800l_sms_mode_t sms_mode)
{
    return ESP_OK;
}

esp_err_t sim800l_sms_list_messages(sim800l_handle_t sim800l_handle, sim800l_sms_list_t sms_list)
{
    return ESP_OK;
}

esp_err_t sim800l_sms_read_message(sim800l_handle_t sim800l_handle, uint32_t index, uint8_t *message)
{
    return ESP_OK;
}

esp_err_t sim800l_sms_send_message(sim800l_handle_t sim800l_handle, uint8_t *number, uint8_t *message)
{
    return ESP_OK;
}

esp_err_t sim800l_sms_delete_message(sim800l_handle_t sim800l_handle, uint32_t delete_flag, uint32_t index)
{
    return ESP_OK;
}

esp_err_t sim800l_sms_store_message(sim800l_handle_t sim800l_handle, uint8_t *message)
{
    return ESP_OK;
}

esp_err_t sim800l_sms_send_stored_message(sim800l_handle_t sim800l_handle, uint32_t index, uint8_t *message)
{
    return ESP_OK;
}