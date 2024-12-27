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
 *     Defines
 */
#define SIM800L_EVENT_CALL_RING_STR "RING"
#define SIM800L_EVENT_CALL_IDENTIFY_STR "+CLIP"
#define SIM800L_EVENT_CALL_NO_CARRIER_STR "NO CARRIER"


/*
 *     Tag
 */
#define SIM800L_CALL_TAG "SIM800L CALL"

sim800l_event_t sim800l_event_call_ring(char **input_args, void *output_data);
sim800l_event_t sim800l_event_call_identify(char **input_args, void *output_data);
sim800l_event_t sim800l_event_call_no_carrier(char **input_args, void *output_data);

/*
 *     Public functions development
 */
sim800l_ret_t sim800l_call_switch(sim800l_handle_t sim800l_handle, bool enable)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);

    /* Check enable */
    if (enable == true)
    {
        /* Register RING callback */
        if (sim800l_register_callback(SIM800L_EVENT_CALL_RING_STR, sim800l_event_call_ring) != ESP_OK)
        {
            ESP_LOGE(SIM800L_CALL_TAG, "sim800l_register_callback failed");
            return SIM800L_RET_ERROR;
        }

        /* Register IDENTIFY callback */
        if (sim800l_register_callback(SIM800L_EVENT_CALL_IDENTIFY_STR, sim800l_event_call_identify) != ESP_OK)
        {
            ESP_LOGE(SIM800L_CALL_TAG, "sim800l_register_callback failed");
            return SIM800L_RET_ERROR;
        }

        /* Register NO CARRIER callback */
        if (sim800l_register_callback(SIM800L_EVENT_CALL_NO_CARRIER_STR, sim800l_event_call_no_carrier) != ESP_OK)
        {
            ESP_LOGE(SIM800L_CALL_TAG, "sim800l_register_callback failed");
            return SIM800L_RET_ERROR;
        }

        return SIM800L_RET_OK;
    }

    /* Unregister RING callback */
    if (sim800l_unregister_callback(SIM800L_EVENT_CALL_RING_STR) != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_unregister_callback failed");
        return SIM800L_RET_ERROR;
    }

    /* Unregister IDENTIFY callback */
    if (sim800l_unregister_callback(SIM800L_EVENT_CALL_IDENTIFY_STR) != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_unregister_callback failed");
        return SIM800L_RET_ERROR;
    }

    /* Unregister NO CARRIER callback */
    if (sim800l_unregister_callback(SIM800L_EVENT_CALL_NO_CARRIER_STR) != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_unregister_callback failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_call_make_call(sim800l_handle_t sim800l_handle, const char *number)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(number) + strlen(SIM800L_COMMAND_CALL) + strlen("\r\n") + 1;
    
    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s%s;\r\n", SIM800L_COMMAND_CALL, number) < 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[10] = {0};

    /* Send AT command */
    esp_err_t ret = sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_make_call failed: %s", esp_err_to_name(ret));
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_make_call failed");
        free(command);
        return SIM800L_RET_ERROR;
    }

    free(command);
    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_call_response(sim800l_handle_t sim800l_handle, sim800l_call_response_t call_response)
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
            return SIM800L_RET_ERROR_SEND_COMMAND;
        }
    }

    /* Send AT command */
    ret = sim800l_out_data(sim800l_handle, (uint8_t *)SIM800L_COMMAND_CALL_ANSWER, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }    

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_call_line_identify(sim800l_handle_t sim800l_handle, bool enable)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);

    /* Response */
    char response[10] = {0};

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_CALL_LINE_ID) + sizeof(char) + strlen("\r\n") + 1;

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s%d\r\n", SIM800L_COMMAND_CALL_LINE_ID, enable) < 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Send AT command */
    esp_err_t ret = sim800l_out_data(sim800l_handle, (uint8_t *)command, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}

/*
 *     SIM800L call event functions
 */
sim800l_event_t sim800l_event_call_ring(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);
    return SIM800L_EVENT_CALL_RING;
}

sim800l_event_t sim800l_event_call_identify(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);

    sim800l_call_identify_t *call_identify = (sim800l_call_identify_t *)output_data;

    /* Get number length */
    char *number_home = strchr(input_args[0], '"') + 1;
    char *number_end = strchr(number_home, '"');
    size_t number_len = number_end - number_home;

    /* Store number in buffer */
    char buffer[number_len + 1];
    if (strncpy(buffer, number_home, number_len) == NULL)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "strncpy failed");
        return -1;
    }

    /* Add byte \0 */
    buffer[number_len] = '\0';

    /* Store number in call_identify */
    if (strncpy(call_identify->number, buffer, strlen(buffer)) == NULL)
    {
        ESP_LOGE(SIM800L_CALL_TAG, "strncpy failed");
        return -1;
    }

    /* Store type in call_identify */
    call_identify->type = atoi(input_args[1]);

    return SIM800L_EVENT_CALL_IDENTIFY;
}

sim800l_event_t sim800l_event_call_no_carrier(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_CALL_TAG, "%s", __func__);
    return SIM800L_EVENT_CALL_NO_CARRIER;
}