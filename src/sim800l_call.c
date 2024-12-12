/*
 * @file sim800l_call.c
 * @author Eduardo Gomes
 * @brief Source file for SIM800L call functions
 * 
 * @copyright MIT
 * 
 */

/*
 *     Includes
 */
#include "sim800l_core.h"
#include "sim800l_call.h"
#include <esp_log.h>
#include <string.h>

/*
 *     Tag
 */
#define SIM800L_CALL_TAG "SIM800L CALL"

/*
 *     Public functions development
 */
esp_err_t sim800l_call_make_call(sim800l_handle_t sim800l_handle, const char *number)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(number) + strlen(SIM800L_COMMAND_CALL) + strlen("\r\n") + 1;
    
    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s%s;\r\n", SIM800L_COMMAND_CALL, number) < 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "Assembly of the command to be sent failed");
        free(command);
        return ESP_FAIL;
    }

    /* Response */
    char response[10] = {0};

    /* Send AT command */
    esp_err_t ret = sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_make_call failed: %s", esp_err_to_name(ret));
        free(command);
        return ret;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_make_call failed");
        free(command);
        return ESP_FAIL;
    }

    free(command);
    return ESP_OK;
}

esp_err_t sim800l_call_answer(sim800l_handle_t sim800l_handle, sim800l_call_response_t call_response)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);

    esp_err_t ret = ESP_FAIL;

    /* Response */
    char response[10] = {0};

    /* Check if call_response is SIM800L_CALL_HANGUP */
    if (call_response == SIM800L_CALL_HANGUP)
    {
        /* Send AT command */
        ret = sim800l_out_data(sim800l_handle, (uint8_t *)SIM800L_COMMAND_CALL_HANGUP, (uint8_t *)response, 1000);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    /* Send AT command */
    ret = sim800l_out_data(sim800l_handle, (uint8_t *)SIM800L_COMMAND_CALL_ANSWER, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed");
        return ESP_FAIL;
    }    

    return ESP_OK;
}

esp_err_t sim800l_call_line_identify(sim800l_handle_t sim800l_handle, sim800l_call_line_id_t call_line_id)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);

    esp_err_t ret = ESP_FAIL;

    /* Response */
    char response[10] = {0};

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_CALL_HANGUP) + sizeof(char) + strlen("\r\n") + 1;

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s%d\r\n", SIM800L_COMMAND_CALL_HANGUP, call_line_id) < 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "Assembly of the command to be sent failed");
        free(command);
        return ESP_FAIL;
    }

    /* Send AT command */
    ret = sim800l_out_data(sim800l_handle, (uint8_t *)command, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        free(command);
        return ret;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed");
        free(command);
        return ESP_FAIL;
    }

    free(command);
    return ESP_OK;
}