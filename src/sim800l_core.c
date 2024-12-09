/*
 * @file sim800l.c
 * @author Eduardo Gomes
 * @brief Source file for SIM800L driver
 * 
 * @copyright MIT
 *
 */

/*
 *     Includes
 */
#include "sim800l_core.h"
#include "sim800l_common.h"
#include "sim800l_urc.h"
#include "sim800l_misc.h"
#include <string.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>

/*
 *     Define
 */
#define SIM800L_ZERO_VALUE              0
#define SIM800L_NULL_VALUE              ((void*)0)
#define SIM800L_GPIO_NC                 GPIO_NUM_NC

#define NUM_OF_PARAMS                   1
#define MAX_PARAMS_SIZE                 100 /* 100*sizeof(uint8_t) = 100 bytes */

#define SIM800L_TASK_STACK_SIZE         4096
#define SIM800L_TASK_PRIORITY           1
#define SIM800L_TASK_NAME               "sim800l_bridge_task"
#define sim800l_bridge_task_DELAY_MS    100

#define SIM800L_UART_BUFFER_SIZE        4096

#define MAX_TOKEN_SIZE                  100


/*
 *     Tag
 */
#define SIM800L_TAG "SIM800L"

/*
 *     Event base definition 
 */
ESP_EVENT_DEFINE_BASE(SIM800L_EVENTS);

/*
 *     Event group bits (Not related to the event base)
 */
static const int SIM800L_EVENT_GROUP_RESPONSE_OK        = BIT14;
static const int SIM800L_EVENT_GROUP_RESPONSE_ERROR     = BIT15;

/*
 *     SIM800L handle
 */
struct sim800l
{
    sim800l_config_t* config;
    TaskHandle_t sim800l_task_handle;
    EventGroupHandle_t sim800l_event_group_handle;
    esp_event_loop_handle_t sim800l_event_loop_handle;
    QueueHandle_t sim800l_queue_handle;
};

/*
 *     Private functions
 */
static esp_err_t sim800l_uart_init(sim800l_handle_t sim800l_handle);
static uint32_t sim800l_uart_send_data(sim800l_handle_t sim800l_handle, uint8_t* data, uint32_t data_len);
static uint32_t sim800l_uart_recv_data(sim800l_handle_t sim800l_handle, uint8_t* data, uint32_t data_len, uint32_t timeout);
static esp_err_t sim800l_post_event(sim800l_handle_t sim800l_handle, sim800l_event_t sim800l_event, void* data);

/*
 *     SIM800L task
 */
static void sim800l_bridge_task(void* args);

/*
 *     Public functions development
 */
esp_err_t sim800l_init(sim800l_handle_t* sim800l_handle, sim800l_config_t* sim800l_config)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    esp_err_t ret = ESP_OK;

    /* Check if config is NULL */
    if ((sim800l_config == NULL) && (sim800l_handle == NULL))
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_config is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /* Create temporary handle */
    sim800l_handle_t sim800l_handle_temp = NULL;
    sim800l_handle_temp = (sim800l_handle_t) calloc(1,sizeof(struct sim800l));
    if (sim800l_handle_temp == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_handle_temp is NULL");
        return ESP_ERR_NO_MEM;
    }
    
    /* Assign config values to temporary handle */
    sim800l_handle_temp->config = sim800l_config;

    /* Setting power pin and reset pin */
    if (sim800l_handle_temp->config->sim800l_pwr_pin == GPIO_NUM_NC)
    {
        /* Configure reset pin */
        ret = gpio_set_direction(sim800l_handle_temp->config->sim800l_rst_pin, GPIO_MODE_OUTPUT);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_direction failed: %s", esp_err_to_name(ret));
            return ret;
        }

        /* Module control by reset pin */
        ret = gpio_set_level(sim800l_handle_temp->config->sim800l_rst_pin, 0);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_level failed: %s", esp_err_to_name(ret));
            return ret;
        }

        /* Wait 105 ms */
        vTaskDelay(105 / portTICK_PERIOD_MS);
    }
    else
    {
        /* Configure pwr pin */
        ret = gpio_set_direction(sim800l_handle_temp->config->sim800l_pwr_pin, GPIO_MODE_OUTPUT);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_direction failed: %s", esp_err_to_name(ret));
            return ret;
        }

        /* Turn off module */
        ret = gpio_set_level(sim800l_handle_temp->config->sim800l_pwr_pin, 1);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_level failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    /* Setting DTR pin */
    ret = gpio_set_direction(sim800l_handle_temp->config->sim800l_dtr_pin, GPIO_MODE_OUTPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "gpio_set_direction failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Set DTR pin low */
    ret = gpio_set_level(sim800l_handle_temp->config->sim800l_dtr_pin, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "gpio_set_level failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Setting RING pin */
    ret = gpio_set_direction(sim800l_handle_temp->config->sim800l_ring_pin, GPIO_MODE_INPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "gpio_set_direction failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Init sim800l uart driver */
    ret = sim800l_uart_init(sim800l_handle_temp);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_uart_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Create event loop */
    esp_event_loop_args_t sim800l_event_loop_args = {
        .queue_size = 1,
        .task_name = SIM800L_NULL_VALUE,
    };

    ret = esp_event_loop_create(&sim800l_event_loop_args, &sim800l_handle_temp->sim800l_event_loop_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "esp_event_loop_create failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Create Event Group */
    sim800l_handle_temp->sim800l_event_group_handle = xEventGroupCreate();
    if (sim800l_handle_temp->sim800l_event_group_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "xEventGroupCreate failed");
        return ESP_ERR_NO_MEM;
    }

    /* Create Queue */
    sim800l_handle_temp->sim800l_queue_handle = xQueueCreate(NUM_OF_PARAMS, MAX_PARAMS_SIZE);
    if (sim800l_handle_temp->sim800l_queue_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "xQueueCreate failed");
        return ESP_ERR_NO_MEM;
    }

    /* Create URC table */
    if (sim800l_urc_create_table() == false)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_urc_create_table failed");
        return ESP_FAIL;
    }

    /* Assign temporary handle to main handle */
    *sim800l_handle = sim800l_handle_temp;

    return ESP_OK;
}

esp_err_t sim800l_start(sim800l_handle_t sim800l_handle)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    esp_err_t ret = ESP_OK;

    /* Check if handle is NULL */
    if (sim800l_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    /* Create task */
    if (xTaskCreate (sim800l_bridge_task,
                                    SIM800L_TASK_NAME, 
                                    SIM800L_TASK_STACK_SIZE,
                                    sim800l_handle,
                                    SIM800L_TASK_PRIORITY,
                                    &sim800l_handle->sim800l_task_handle) != pdPASS)
    {
        ESP_LOGE(SIM800L_TAG, "xTaskCreate failed");
        return ESP_FAIL;
    }

    /* Check PWRKEY */
    if (sim800l_handle->config->sim800l_pwr_pin == GPIO_NUM_NC)
    {
        /* Module control by reset pin */
        ret = gpio_set_level(sim800l_handle->config->sim800l_rst_pin, 1);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_level failed: %s", esp_err_to_name(ret));
            return ret;
        }

        /* Reset delay */
        vTaskDelay(3000/portTICK_PERIOD_MS);
    }   
    else
    {
        /* Turn on module */
        ret = gpio_set_level(sim800l_handle->config->sim800l_pwr_pin, 0);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_level failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    /* AT command test */
    if (sim800l_command_AT(sim800l_handle) != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_out_data failed");
        return ESP_FAIL;
    }

    /* Wait for SIM800L to be ready */
    BaseType_t events_start = xEventGroupWaitBits (sim800l_handle->sim800l_event_group_handle, 
                                                    SIM800L_URC_EVENT_RDY |
                                                    SIM800L_URC_EVENT_CFUN_FULL |
                                                    SIM800L_URC_EVENT_CPIN_READY |
                                                    SIM800L_URC_EVENT_CALL_READY |
                                                    SIM800L_URC_EVENT_SMS_READY,
                                                    pdTRUE, 
                                                    pdTRUE, 
                                                    60000/portTICK_PERIOD_MS);
    if(events_start == (SIM800L_URC_EVENT_RDY |
                        SIM800L_URC_EVENT_CFUN_FULL |
                        SIM800L_URC_EVENT_CPIN_READY |
                        SIM800L_URC_EVENT_CALL_READY |
                        SIM800L_URC_EVENT_SMS_READY ))
    {

        ESP_LOGI(SIM800L_TAG, "SIM800L is ready");
        return ESP_OK;
    }

    ESP_LOGE(SIM800L_TAG, "SIM800L is not ready");
    return ESP_FAIL;
}

esp_err_t sim800l_register_event(sim800l_handle_t sim800l_handle, sim800l_event_t sim800l_event, esp_event_handler_t sim800l_event_handler, void *sim800l_event_handler_arg)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    esp_err_t ret = ESP_OK;

    /* Check if handle is NULL */
    if (sim800l_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /* Register event handler */
    ret = esp_event_handler_register (sim800l_handle->sim800l_event_loop_handle, 
                                    (int32_t)SIM800L_EVENTS, 
                                    sim800l_event_handler, 
                                    sim800l_event_handler_arg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "esp_event_handler_register failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ret;
}

esp_err_t sim800l_out_data(sim800l_handle_t sim800l_handle, uint8_t *command, uint8_t *response, uint32_t timeout)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    /* Check if handle is NULL */
    if (sim800l_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /* Check if data_set is NULL */
    if (command != NULL)
    {
        /* Send AT command */
        if (sim800l_uart_send_data(sim800l_handle, command, strlen((char *)command)) < 1)
        {
            ESP_LOGE(SIM800L_TAG, "uart_write_bytes failed");
        }
    }

    /* Check if response is NULL */
    if (response != NULL)
    {
        /* Send Queue */
        if (xQueueSend(sim800l_handle->sim800l_queue_handle, response, 0) != pdPASS)
        {
            ESP_LOGE(SIM800L_TAG, "xQueueSend failed"); 
            return ESP_FAIL;
        }

        /* Wait for response */
        BaseType_t event_response = xEventGroupWaitBits(sim800l_handle->sim800l_event_group_handle,
                                                        SIM800L_EVENT_GROUP_RESPONSE_OK | 
                                                        SIM800L_EVENT_GROUP_RESPONSE_ERROR,
                                                        pdTRUE,
                                                        pdFALSE,
                                                        timeout);
        if (event_response & SIM800L_EVENT_GROUP_RESPONSE_OK)
        {
            return ESP_OK;
        }
        else if (event_response & SIM800L_EVENT_GROUP_RESPONSE_ERROR)
        {
            ESP_LOGI(SIM800L_TAG, "Response ERROR");
            return ESP_FAIL;
        }

        ESP_LOGE(SIM800L_TAG, "Timeout"); 
        return ESP_FAIL;
    }

    return ESP_ERR_INVALID_ARG;
}

/*
 *     Private functions development
 */
static esp_err_t sim800l_uart_init(sim800l_handle_t sim800l_handle)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    esp_err_t ret = ESP_OK;
    
    /* Init sim800l uart driver */
    ret = uart_driver_install (sim800l_handle->config->sim800l_uart_port,           /* UART port number */
                                                 SIM800L_UART_BUFFER_SIZE,                            /* UART RX buffer size */
                                                 SIM800L_ZERO_VALUE,                                  /* UART TX buffer size */
                                                 SIM800L_ZERO_VALUE,                                  /* UART event queue size */
                                                 SIM800L_NULL_VALUE,                                  /* UART event queue handle */
                                                 SIM800L_ZERO_VALUE);                                 /* UART interrupt allocation flags */
    if (ret != ESP_OK)
    {   
        ESP_LOGE(SIM800L_TAG, "UART driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Config sim800l uart driver */
    ret = uart_param_config (sim800l_handle->config->sim800l_uart_port,    /* UART port number */
                            &sim800l_handle->config->sim800l_uart_config); /* UART config */
    if (ret != ESP_OK) 
    {
        ESP_LOGE(SIM800L_TAG, "UART param config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    /* Set sim800l uart pins */
    ret = uart_set_pin (sim800l_handle->config->sim800l_uart_port,   /* UART port number */
                        sim800l_handle->config->sim800l_uart_tx_pin, /* UART TX pin */
                        sim800l_handle->config->sim800l_uart_rx_pin, /* UART RX pin */
                        SIM800L_GPIO_NC,                             /* UART RTS pin */
                        SIM800L_GPIO_NC);                            /* UART CTS pin */
    if (ret != ESP_OK) 
    {
        ESP_LOGE(SIM800L_TAG, "UART set pin failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ret;
}

static uint32_t sim800l_uart_send_data(sim800l_handle_t sim800l_handle, uint8_t *data, uint32_t data_len)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    /* Send data to sim800l uart */
    return uart_write_bytes (sim800l_handle->config->sim800l_uart_port, data, data_len);
}

static uint32_t sim800l_uart_recv_data(sim800l_handle_t sim800l_handle, uint8_t *data, uint32_t data_len, uint32_t timeout)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    return uart_read_bytes (sim800l_handle->config->sim800l_uart_port, data, data_len, timeout / portTICK_PERIOD_MS);
}

static esp_err_t sim800l_post_event(sim800l_handle_t sim800l_handle, sim800l_event_t sim800l_event, void* data)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    esp_err_t ret = ESP_OK;

    /* Check if handle is NULL */
    if (sim800l_handle == 0)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /* Check if event is NULL */
    if (sim800l_event == 0)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_event is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    sim800l_event_data_t sim800l_event_data = {
        .sim800l_handle = sim800l_handle,
        .ptr = data
    };

    /* Post event */
    ret = esp_event_post_to (sim800l_handle->sim800l_event_loop_handle, /* Event loop handle */
                                                SIM800L_EVENTS,                            /* Events base */
                                                sim800l_event,                             /* Event */
                                                &sim800l_event_data,                       /* Event data */
                                                sizeof(sim800l_event_data),                /* Event data size */
                                                portMAX_DELAY);                            /* Timeout */
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "esp_event_post_to failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Run event loop */
    ret = esp_event_loop_run(sim800l_handle->sim800l_event_loop_handle, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "esp_event_loop_run failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ret;
}

/*
 * SIM800L interpreter task
 *
 * @brief This task is used to interpret the data received from the SIM800L module.
 * 
 */
static void sim800l_bridge_task(void *args)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    /* Create temporary handle */
    sim800l_handle_t sim800l_handle = (sim800l_handle_t)args;

    while (true)
    {
        uint8_t expected_response[MAX_PARAMS_SIZE] = {0};

        /* Read data from sim800l uart */
        uint8_t data[256] = {0};
        if (sim800l_uart_recv_data(sim800l_handle, data, sizeof(data), 50) > 0)
        {
            // ESP_LOGI(SIM800L_TAG, "Received data: %s", data);

            /* Receive Queue */
            if (xQueueReceive(sim800l_handle->sim800l_queue_handle, expected_response, 0) == pdTRUE)
            {
                /* Compare data */
                if (strnstr((const char*)expected_response, (const char*)data, strlen((const char*)expected_response)) == NULL)
                {
                    /* Set event to notify that data has been received */
                    xEventGroupSetBits(sim800l_handle->sim800l_event_group_handle, SIM800L_EVENT_GROUP_RESPONSE_OK);
                }
                else
                {
                    /* Set event to notify that data has been received */
                    xEventGroupSetBits(sim800l_handle->sim800l_event_group_handle, SIM800L_EVENT_GROUP_RESPONSE_ERROR);
                }
            }

            /* Extract tokens */
            char *token = calloc(1, MAX_TOKEN_SIZE * sizeof(char));
            token = strtok((char*)data, "\r\n");

            do
            {
                if (token != NULL)
                {
                    char urc[25] = {0};

                    char **urc_args = calloc(1, 5 * sizeof(char*));

                    /* Check if the token is a URC in format: +<token>:*/
                    if (token[0] == '+')
                    {
                        /* Extract URC (+<urc>:args) */
                        token = strtok(token, ":");
                        strncpy(urc, token, strlen(token));

                        int i = 0;
                        token = strtok(NULL, ",");
                        do
                        {
                            urc_args[i] = strdup(token);

                            token = strtok(NULL, ",");
                            i++;
                        } while ((token != NULL) && (i < 5));
                    }
                    else
                    {
                        /* Extract simple URC */
                        strncpy(urc, token, strlen(token));
                    }

                    /* Interpret URC */
                    sim800l_urc_event_t urc_event = sim800l_urc_interpret((const char *)urc, urc_args);
                    if (urc_event != -1)
                    {
                        xEventGroupSetBits(sim800l_handle->sim800l_event_group_handle, urc_event);
                    }

                    free(urc_args);
                }

                /* Get next token */
                token = strtok(NULL, "\r\n");
            } while (token);

            free(token);
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}