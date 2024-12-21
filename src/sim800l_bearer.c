/*
 * @file sim800l_bearer.c
 * @author Eduardo Gomes
 * @brief Source file for SIM800L bearer functions
 * 
 * @copyright MIT
 *
 */

/*
 *     Includes
 */
#include "sim800l_core.h"
#include "sim800l_bearer.h"
#include "sim800l_urc.h"
#include "sim800l_misc.h"
#include <string.h>

/*
 *     Tag
 */
#define SIM800L_BEARER_TAG "SIM800L BEARER"

/*
 *     Functions
 */

sim800l_ret_t sim800l_bearer_switch(sim800l_handle_t sim800l_handle, bool bearer_state)
{
    ESP_LOGD(SIM800L_BEARER_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_BEARER) + 4*sizeof(char) + strlen("\r\n") + 1; /* cmd=d,d\r\n */

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=%d,1\r\n", SIM800L_COMMAND_BEARER, bearer_state) < 0)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[10] = {0};
    
    /* Send AT command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 85000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    /* Check response */
    if (strnstr((char *)response, "OK", sizeof(response)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_bearer_query(sim800l_handle_t sim800l_handle, sim800l_bearer_t *bearer)
{
    ESP_LOGD(SIM800L_BEARER_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_BEARER) + 4*sizeof(char) + strlen("\r\n") + 1; /* cmd=d,d\r\n */

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=2,1\r\n", SIM800L_COMMAND_BEARER) < 0)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[10] = {0};
    
    /* Send AT command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    /* Check response */
    if (strnstr((char *)response, "ERROR", sizeof(response)) != NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract CID */
    char *token = strtok((char *)response, "M,");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting CID failed");
        return SIM800L_RET_ERROR;
    }

    char cid_string[4] = {0};
    if (strncpy(cid_string, token, strlen(token)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Store CID failed");
        return SIM800L_RET_ERROR;
    }

    /* Set CID */
    bearer->cid = atoi(cid_string);

    /* Extract status */
    token = strtok(NULL, ",");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting status failed");
        return SIM800L_RET_ERROR;
    }

    char status_string[2] = {0};
    if (strncpy(status_string, token, strlen(token)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Store status failed");
        return SIM800L_RET_ERROR;
    }

    /* Set status */
    bearer->status = atoi(status_string);

    /* Extract ipv4 */
    token = strtok(NULL, ",");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting ipv4 failed");
        return SIM800L_RET_ERROR;
    }

    /* Set ipv4 */    
    if (strncpy(bearer->ipv4, token, strlen(token)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Store ipv4 failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_bearer_set_param(sim800l_handle_t sim800l_handle, const char* param, const char* value)
{
    ESP_LOGD(SIM800L_BEARER_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_BEARER) 
                            + strlen(param) 
                            + strlen(value) 
                            + 10*sizeof(char) 
                            + strlen("\r\n") 
                            + 1; /* cmd=d,d,\"param\",\"value\"\r\n */

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=3,1,\"%s\",\"%s\"\r\n", SIM800L_COMMAND_BEARER, param, value) < 0)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[10] = {0};
    
    /* Send AT command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    /* Check response */
    if (strnstr((char *)response, "OK", sizeof(response)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_bearer_get_param(sim800l_handle_t sim800l_handle, sim800l_bearer_param_t *param)
{
    ESP_LOGD(SIM800L_BEARER_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_BEARER) + 4*sizeof(char) + strlen("\r\n") + 1; /* cmd=d,d\r\n */

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=4,1\r\n", SIM800L_COMMAND_BEARER) < 0)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[100] = {0};
    
    /* Send AT command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    /* Extract Contype */
    char *token = strtok(response, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting Contype failed");
        return SIM800L_RET_ERROR;
    }
    
    char *temp = token + strlen("CONTYPE: ");

    if (strncpy(param->contype, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Store temp Contype failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract APN */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting APN failed");
        return SIM800L_RET_ERROR;
    }

    temp = token + strlen("APN: ");

    if (strncpy(param->apn, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Store temp APN failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract Phone Number */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting Phone Number failed");
        return SIM800L_RET_ERROR;
    }

    temp = token + strlen("PHONENUM: ");

    if (strncpy(param->phonenum, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Store temp Phone Number failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract User */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting User failed");
        return SIM800L_RET_ERROR;
    }

    temp = token + strlen("USER: ");

    if (strncpy(param->user, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Store temp User failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract PWD */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting PWD failed");
        return SIM800L_RET_ERROR;
    }

    temp = token + strlen("PWD: ");

    if (strncpy(param->pwd, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Store temp PWD failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract Rate */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_BEARER_TAG, "Extracting Rate failed");
        return SIM800L_RET_ERROR;
    }

    temp = token + strlen("RATE: ");

    /* Convert to int */
    param->rate = atoi(temp);


    return SIM800L_RET_OK;
}