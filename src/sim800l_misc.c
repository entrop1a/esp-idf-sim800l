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

/*
 *     Tag
 */
#define SIM800L_MISCELLANEOUS_TAG "SIM800L MISCELLANEOUS"

/*
 *     Public functions development
 */
esp_err_t sim800l_command_AT(sim800l_handle_t sim800l_handle)
{
    ESP_LOGD(SIM800L_MISCELLANEOUS_TAG, "%s", __func__);

    /* Returns */
    esp_err_t esp_ret = ESP_OK;

    /* Send AT command */
    esp_ret = sim800l_out_data(sim800l_handle, (uint8_t *)SIM800L_COMMAND_AT, (uint8_t *)"OK\r\n", 1000);
    if (esp_ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_MISCELLANEOUS_TAG, "sim800l_command_AT failed: %s", esp_err_to_name(esp_ret));
        return esp_ret;
    }

    return esp_ret;
}

esp_err_t sim800l_command_set_baud_rate(sim800l_handle_t sim800l_handle, uint32_t baud_rate)
{
    ESP_LOGD(SIM800L_MISCELLANEOUS_TAG, "%s", __func__);

    /* Returns */
    int32_t int_ret = 0;
    esp_err_t esp_ret = ESP_OK;

    /*
     * strlen("AT+IPR=") = 7
     * strlen("460800") = 6, This is the maximum possible value for the baud rate.
     * strlen("\r\n") = 2
     * Total = 15 bytes
     */
    uint8_t *set_baud_rate_command = calloc(15, sizeof(uint8_t));

    /* Assembly of the command to be sent */
    int_ret = snprintf((char*)set_baud_rate_command, 15, "%s%lu\r\n", SIM800L_COMMAND_SET_BAUD_RATE, baud_rate);
    if (int_ret < 0)
    {
        ESP_LOGE(SIM800L_MISCELLANEOUS_TAG, "Assembly of the command to be sent failed");
        free(set_baud_rate_command);
        esp_ret = ESP_FAIL;
        return esp_ret;
    }
    
    /* Send AT command */
    esp_ret = sim800l_out_data(sim800l_handle, set_baud_rate_command, (uint8_t *)"OK\r\n", 1000);
    if (esp_ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_MISCELLANEOUS_TAG, "Send AT command failed");
        free(set_baud_rate_command);
        return esp_ret;
    }

    free(set_baud_rate_command);
    return esp_ret;
    
}