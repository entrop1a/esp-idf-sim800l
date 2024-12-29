/*
 * @file sim800l_http.c
 * @author Eduardo Gomes
 * @brief Source file for SIM800L HTTP functions
 * 
 * @copyright MIT
 *
 */

/*
 *     Includes
 */
#include "sim800l_core.h"
#include "sim800l_http.h"
#include "sim800l_common.h"
#include "sim800l_misc.h"
#include <string.h>

#define SIM800L_HTTP_TAG "SIM800L HTTP"

#define SIM800L_EVENT_HTTP_ACTION_STR "+HTTPACTION"

sim800l_event_t sim800l_event_http_action(char **input_args, void *output_data);

sim800l_ret_t sim800l_http_switch(sim800l_handle_t sim800l_handle, bool enable)
{
    ESP_LOGD(SIM800L_HTTP_TAG, "%s", __func__);

    char command[16] = {0};

    if (enable)
    {
        /* Register callback */
        if (sim800l_register_callback(SIM800L_EVENT_HTTP_ACTION_STR, sim800l_event_http_action) != ESP_OK)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "sim800l_register_callback failed");
            return SIM800L_RET_ERROR;
        }

        /* Assembly of the command to be sent */
        if (strncpy(command, SIM800L_COMMAND_HTTP_INIT, strlen(SIM800L_COMMAND_HTTP_INIT)) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;
        }
    }
    else
    {
        /* Unregister callback */
        if (sim800l_unregister_callback(SIM800L_EVENT_HTTP_ACTION_STR) != ESP_OK)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "sim800l_unregister_callback failed");
            return SIM800L_RET_ERROR;
        }

        /* Assembly of the command to be sent */
        if (strncpy(command, SIM800L_COMMAND_HTTP_TERMINATE, strlen(SIM800L_COMMAND_HTTP_TERMINATE)) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }

    /* Response */
    char response[10] = {0};

    /* Send AT command */
    esp_err_t ret = sim800l_out_data(sim800l_handle, (uint8_t *)command, (uint8_t *)response, 2000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "sim800l_out_data failed: %s", esp_err_to_name(ret));
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    if (strncmp(response, "OK", strlen("OK")) != 0)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "sim800l_out_data failed");
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_http_set_param(sim800l_handle_t sim800l_handle, sim800l_http_param_tag_t param, const char *value)
{
    ESP_LOGD(SIM800L_HTTP_TAG, "%s", __func__);

    char http_parameter[16] = {0};

    /* Check param */
    if (param == SIM800L_HTTP_PARAM_CID)
    {
        if (strncpy(http_parameter, "CID", strlen("CID")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_URL)
    {
        if (strncpy(http_parameter, "URL", strlen("URL")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_UA)
    {
        if (strncpy(http_parameter, "UA", strlen("UA")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_PROIP)
    {
        if (strncpy(http_parameter, "PROIP", strlen("PROIP")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_PROPORT)
    {
        if (strncpy(http_parameter, "PROPORT", strlen("PROPORT")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_REDIR)
    {
        if (strncpy(http_parameter, "REDIR", strlen("REDIR")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_BREAK)
    {
        if (strncpy(http_parameter, "BREAK", strlen("BREAK")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_BREAKEND)
    {
        if (strncpy(http_parameter, "BREAKEND", strlen("BREAKEND")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_TIMEOUT)
    {
        if (strncpy(http_parameter, "TIMEOUT", strlen("TIMEOUT")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_CONTENT)
    {
        if (strncpy(http_parameter, "CONTENT", strlen("CONTENT")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else if (param == SIM800L_HTTP_PARAM_USERDATA)
    {
        if (strncpy(http_parameter, "USERDATA", strlen("USERDATA")) == NULL)
        {
            ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
            return SIM800L_RET_ERROR_BUILD_COMMAND;   
        }
    }
    else
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Invalid param");
        return SIM800L_RET_INVALID_ARG;
    }

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_HTTP_PARAM) 
                            + strlen(http_parameter) 
                            + strlen(value) 
                            + 5*sizeof(char) 
                            + strlen("\r\n") 
                            + 1; /* strlen("cmd=\"%s\",\"%s\"\r\n") = 6*sizeof(char) */

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=\"%s\",\"%s\"\r\n", SIM800L_COMMAND_HTTP_PARAM, http_parameter, value) < 0)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[100] = {0};

    /* Send command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 60000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    /* Check response */
    if (strnstr(response, "OK", sizeof(response)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Error");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_http_get_param(sim800l_handle_t sim800l_handle, sim800l_http_param_t *param)
{
    ESP_LOGD(SIM800L_HTTP_TAG, "%s", __func__);

    /* Get command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_HTTP_PARAM) 
                            + sizeof(char) 
                            + strlen("\r\n") 
                            + 1; /* strlen("cmd?\r\n") = 6*sizeof(char) */

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s?\r\n", SIM800L_COMMAND_HTTP_PARAM) < 0)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[256] = {0};

    /* Send command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 60000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    /* Check response */
    if (strnstr(response, "ERROR", sizeof(response)) != NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Error");
        return SIM800L_RET_ERROR;
    }

    /* Extract CID */
    char *token = strtok(response, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting Contype failed");
        return SIM800L_RET_ERROR;
    }
    
    char *temp = token + strlen("CID: ");

    param->cid = (uint32_t)atoi(temp);

    /* Extract URL */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting URL failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("URL: ");

    if (strncpy(param->url, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Store temp URL failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract UA */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting User-Agent failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("UA: ");

    if (strncpy(param->ua, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Store temp User-Agent failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract PROIP */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting PROIP failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("PROIP: ");

    if (strncpy(param->proip, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Store temp PROIP failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract PROPORT */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting PROPORT failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("PROPORT: ");

    if (strncpy(param->proport, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Store temp PROPORT failed");
        return SIM800L_RET_ERROR;
    }
    
    /* Extract REDIR */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting REDIR failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("REDIR: ");

    param->redir = (uint32_t)atoi(temp);

    /* Extract BREAK */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting BREAK failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("BREAK: ");

    if (strncpy(param->break_, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Store temp BREAK failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract BREAKEND */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting BREAKEND failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("BREAKEND: ");

    if (strncpy(param->breakend, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Store temp BREAKEND failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract TIMEOUT */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting TIMEOUT failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("TIMEOUT: ");

    param->timeout = (uint32_t)atoi(temp);
    
    /* Extract CONTENT */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting CONTENT failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("CONTENT: ");

    if (strncpy(param->content, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Store temp CONTENT failed");
        return SIM800L_RET_ERROR;
    }

    /* Extract USERDATA */
    token = strtok(NULL, "\r\n");
    if (token == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Extracting USERDATA failed");
        return SIM800L_RET_ERROR;
    }
    
    temp = token + strlen("USERDATA: ");

    if (strncpy(param->userdata, temp, strlen(temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Store temp USERDATA failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_http_action(sim800l_handle_t sim800l_handle, sim800l_http_method_t method)
{
    ESP_LOGD(SIM800L_HTTP_TAG, "%s", __func__);

    /* Command length */
    uint32_t command_length = strlen(SIM800L_COMMAND_HTTP_ACTION) + 2*sizeof(char) + strlen("\r\n") + 1; /* cmd=d\r\n */

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    /* Assembly of the command to be sent */
    if (snprintf((char*)command, command_length, "%s=%d\r\n", SIM800L_COMMAND_HTTP_ACTION, method) < 0)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[128] = {0};
    
    /* Send AT command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "sim800l_call_answer failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    /* Check response */
    if (strnstr(response, "ERROR", sizeof(response)) != NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "sim800l_call_answer failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}

sim800l_ret_t sim800l_http_read(sim800l_handle_t sim800l_handle, uint32_t start_addr, size_t length, uint8_t *buffer)
{
    ESP_LOGD(SIM800L_HTTP_TAG, "%s", __func__);

    if (buffer == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Buffer is NULL");
        return SIM800L_RET_INVALID_ARG;
    }

    char start_addr_str[8] = {0};
    if (snprintf(start_addr_str, sizeof(start_addr_str), "%lu", start_addr) < 0)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Assembly of the command to be sent failed");
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    char length_str[8] = {0};
    if (snprintf(length_str, sizeof(length_str), "%d", length) < 0)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Assembly of the command to be sent failed");
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    uint32_t command_length = strlen(SIM800L_COMMAND_HTTP_READ) + 2 * sizeof(char) + strlen(start_addr_str) + strlen(length_str) + strlen("\r\n") + 1; /* cmd=d,d\r\n */

    /* Allocate dinamic memory */
    uint8_t *command = calloc(command_length, sizeof(uint8_t));
    if (command == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Memory allocation failed");
        return SIM800L_RET_ERROR_MEM;
    }

    ESP_LOGI(SIM800L_HTTP_TAG, "start_addr_str: %s", start_addr_str);

    /* Assembly of the command to be sent */
    if (snprintf((char *)command, command_length, "%s=%s,%s\r\n", SIM800L_COMMAND_HTTP_READ, start_addr_str, length_str) < 0)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Assembly of the command to be sent failed");
        free(command);
        return SIM800L_RET_ERROR_BUILD_COMMAND;
    }

    /* Response */
    char response[512] = {0};

    /* Send command */
    if (sim800l_out_data(sim800l_handle, command, (uint8_t *)response, 1000) != ESP_OK)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Command sending failed");
        free(command);
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    free(command);

    /* Check response */
    if (strncmp(response, "ERROR", strlen("ERROR")) == 0)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "Command sending failed");
        return SIM800L_RET_ERROR_SEND_COMMAND;
    }

    char *response_start = strnstr(response, "\r\n", strlen(response)) + 1;
    char *response_end = strnstr(response_start, "OK", strlen(response_start));
    size_t response_length = response_end - response_start;
    char buffer_temp[response_length + 1];
    memset(buffer_temp, 0, response_length + 1);
    if (strncpy(buffer_temp, response_start, response_length) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
        return SIM800L_RET_ERROR;
    }
    buffer[response_length] = '\0';

    ESP_LOGI(SIM800L_HTTP_TAG, "Response: %s", buffer_temp);

    if (strncpy((char *)buffer, buffer_temp, strlen(buffer_temp)) == NULL)
    {
        ESP_LOGE(SIM800L_HTTP_TAG, "strncpy failed");
        return SIM800L_RET_ERROR;
    }

    return SIM800L_RET_OK;
}


/*
 *     SIM800L call event functions
 */
sim800l_event_t sim800l_event_http_action(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_HTTP_TAG, "%s", __func__);

    sim800l_http_action_t *action = (sim800l_http_action_t *)output_data;

    /* Get Method */
    action->method = atoi(input_args[0]);

    /* Get http code */
    action->http_code = atoi(input_args[1]);

    /* Get content length */
    action->content_length = atoi(input_args[2]);

    return SIM800L_EVENT_HTTP_ACTION;
}