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
#include <string.h>

/*
 *     Tag
 */
#define SIM800L_SMS_TAG "SIM800L SMS"

/*
 *     Public functions development
 */
esp_err_t sim800l_sms_sel_mode(sim800l_handle_t sim800l_handle, sim800l_sms_mode_t sms_mode)
{
    ESP_LOGD(SIM800L_SMS_TAG, "%s", __func__);

    esp_err_t ret = ESP_FAIL;

    /* Response */
    char response[10] = {0};

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_SMS_MODE) + sizeof(char) + strlen("\r\n") + 1;

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s%d\r\n", SIM800L_COMMAND_SMS_MODE, sms_mode) < 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Assembly of the command to be sent failed");
        free(command);
        return ESP_FAIL;
    }

    /* Send AT command */
    ret = sim800l_out_data(sim800l_handle, (uint8_t *)command, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        free(command);
        return ret;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        free(command);
        return ESP_FAIL;
    }

    free(command);
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