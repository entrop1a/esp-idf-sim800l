/*
 * @file sim800l_general.c
 * @author Eduardo Gomes
 * @brief Source file for SIM800L general functions
 * 
 * @copyright MIT
 * 
 */

/*
 *     Includes
 */
#include "sim800l_core.h"
#include "sim800l_misc.h"
#include <esp_log.h>
#include <string.h>

/*
 *     Tag
 */
#define SIM800L_MISC_TAG "SIM800L MISC"

/*
 *     Public functions development
 */
sim800l_ret_t sim800l_command_AT(sim800l_handle_t sim800l_handle)
{
    ESP_LOGD(SIM800L_MISC_TAG, "%s", __func__);

    char response[10] = {0};

    /* Send AT command */
    esp_err_t esp_ret = sim800l_out_data(sim800l_handle, (uint8_t *)SIM800L_COMMAND_AT, (uint8_t *)response, 1000);
    if (esp_ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_MISC_TAG, "sim800l_command_AT failed: %s", esp_err_to_name(esp_ret));
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    if (strnstr(response, "OK", 10) == NULL)
    {
        ESP_LOGE(SIM800L_MISC_TAG, "sim800l_command_AT failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}