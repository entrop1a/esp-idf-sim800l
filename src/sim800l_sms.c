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
#include "sim800l_urc.h"
#include <esp_log.h>
#include <string.h>

/*
 *     Tag
 */
#define SIM800L_SMS_TAG "SIM800L SMS"

/*
 *     Public functions development
 */
sim800l_ret_t sim800l_sms_set_mode(sim800l_handle_t sim800l_handle, sim800l_sms_mode_t sms_mode)
{
    ESP_LOGD(SIM800L_SMS_TAG, "%s", __func__);

    esp_err_t ret = ESP_FAIL;

    /* Response */
    char response[10] = {0};

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_SMS_MODE) + sizeof(char) + sizeof(sim800l_sms_mode_t) + strlen("\r\n") + 1;

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=%d\r\n", SIM800L_COMMAND_SMS_MODE, sms_mode) < 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Assembly of the command to be sent failed");
        free(command);
        command = NULL;
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Send AT command */
    ret = sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        free(command);
        command = NULL;
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        free(command);
        command = NULL;
        return SIM800L_RET_ERROR;
    }

    free(command);
    command = NULL;
    return SIM800L_RET_OK;
}

sim800l_sms_mode_t sim800l_sms_get_mode(sim800l_handle_t sim800l_handle)
{
    ESP_LOGD(SIM800L_SMS_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_SMS_MODE) + sizeof(char) + strlen("\r\n") + 1;

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR;
    }

    /* Assembly of the command to be sent */
    if (sniprintf((char *)command, command_length, "%s?\r\n", SIM800L_COMMAND_SMS_MODE) < 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR;
    }

    /* Response */
    char response[10] = {0};

    /* Send AT command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR;
    }

    if(strncmp(response, "1", strlen("1")) == 0)
    {
        free(command);
        return SIM800L_SMS_MODE_TEXT;
    }
    else if(strncmp(response, "0", strlen("0")) == 0)
    {
        free(command);
        return SIM800L_SMS_MODE_PDU;
    }

    free(command);
    return SIM800L_RET_ERROR;
}

sim800l_ret_t sim800l_sms_read_message(sim800l_handle_t sim800l_handle, uint32_t index, sim800l_sms_message_t *message)
{
    ESP_LOGD(SIM800L_SMS_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_SMS_READ) + 6*sizeof(char) + 1; /* strlen("=%d,0\r\n") = 5*sizeof(char) */

    /* Allocate dinamic memory */
    uint8_t *command = (uint8_t *)calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=%lu,0\r\n", SIM800L_COMMAND_SMS_READ, index) < 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[256] = {0};

    /* Send AT command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    if (strncmp(response, "ERROR", strlen("ERROR")) == 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }
    
    /* Remove message type */
    char *result_find = strchr((const char *)response, ',');
    if (result_find != NULL)
    {
        if (strncpy(response, result_find+1, 256) == NULL)
        {
            ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
            return SIM800L_RET_ERROR;
        }
    }

    /* Extract source number */
    char *token = strtok(response, ",");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    /* Remove " " & copy to buffer */
    if (strncpy(message->source_number, token + 1, strlen(token + 1) - 1) == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    /* Skip '","' */
    token = strtok(NULL, ",");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract UTC */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    /* Remove " " & copy to buffer */
    if (strncpy(message->utc_date, token + 1, strlen(token + 1) - 1) == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract message */   
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    /* copy to buffer */
    if (strncpy(message->payload, token, strlen(token)) == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    return ESP_OK;
}

sim800l_ret_t sim800l_sms_send_message(sim800l_handle_t sim800l_handle, const char *number, const char *message)
{
    ESP_LOGD(SIM800L_SMS_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_SMS_SEND) + 2*sizeof(char) + strlen(number) + strlen("\r\n") + 1; /* strlen("=%d,%d\r\n") = 6*sizeof(char) */

    /* Allocate dinamic memory */
    uint8_t *command = (uint8_t *)calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=\"%s\"\r\n", SIM800L_COMMAND_SMS_SEND, number) < 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[10] = {0};

    /* Send AT command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);
    command = NULL;

    if (strncmp(response, ">", strlen(">")) != 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    memset(response, 0, sizeof(response));

    uint32_t message_length = strlen(message) + sizeof(char)  + strlen("\r\n") + 1;

    /* Allocate dinamic memory for message */
    uint8_t *message_buffer = (uint8_t *)calloc(message_length, sizeof(uint8_t));
    if (message == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the message to be sent */
    if (snprintf((char*)message_buffer, message_length, "%s\r\n", message) < 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Assembly of the message to be sent failed");
        free(message_buffer);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Send message */
    if (sim800l_out_data(sim800l_handle, (uint8_t *)message, NULL, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(message_buffer);
    message_buffer = NULL;

    /* Terminate message */
    uint8_t end_byte = 0x1A;

    char end_byte_buffer[4] = {0};

    /* Assembly of the message to be sent */
    if (snprintf(end_byte_buffer, 4, "%c\r\n", (char)end_byte) < 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Assembly of the message to be sent failed");
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    if (sim800l_out_data_event(sim800l_handle, (uint8_t *)end_byte_buffer, SIM800L_URC_EVENT_SMS_SEND, 60000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }
                                        
    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_sms_delete_message(sim800l_handle_t sim800l_handle, uint32_t delete_flag, uint32_t index)
{
    ESP_LOGD(SIM800L_SMS_TAG, "%s", __func__);

    /* Check if flag is valid */
    if (delete_flag > 4)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "delete_flag is invalid");
        return SIM800L_RET_INVALID_ARG;
    }

    /* Check if index is valid */
    if (index == 0 && delete_flag == 4)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "index is invalid");
        return SIM800L_RET_INVALID_ARG;
    }

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_SMS_DEL) + 2*sizeof(char) + 2*sizeof(uint32_t) + strlen("\r\n") + 1; /* strlen("=%d,%d\r\n") = 6*sizeof(char) */
    
    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=%lu,%lu\r\n", SIM800L_COMMAND_SMS_DEL, index, delete_flag) < 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[10] = {0};

    /* Send AT command */
    esp_err_t ret = sim800l_out_data(sim800l_handle, (uint8_t *)command, (uint8_t *)response, 1000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_SMS_TAG, "sim800l_call_answer failed: %s", esp_err_to_name(ret));
        free(command);
        return SIM800L_RET_ERROR;
    }

    return ESP_OK;
}
