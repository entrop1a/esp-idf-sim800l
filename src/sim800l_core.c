/*
 * @file sim800l.c
 * @author Eduardo Gomes
 * @brief Soevente file for SIM800L driver
 * 
 * @copyright MIT
 *
 */

/*
 *     Includes
 */
#include "sim800l_core.h"
#include "sim800l_common.h"
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
#define MAX_PARAMS_SIZE                 512 /* 100*sizeof(uint8_t) = 100 bytes */

#define SIM800L_TASK_STACK_SIZE         8192
#define SIM800L_TASK_PRIORITY           1
#define SIM800L_TASK_NAME               "sim800l_bridge_task"
#define sim800l_bridge_task_DELAY_MS    100

#define SIM800L_UART_BUFFER_SIZE        4096

#define MAX_TOKEN_SIZE                  512

#define TABLE_SIZE 8


/*
 *     Tag
 */
#define SIM800L_TAG "SIM800L CORE"

/*
 *     Event base definition 
 */
ESP_EVENT_DEFINE_BASE(SIM800L_EVENTS);

/*
 *     SIM800L handle
 */
struct sim800l
{
    sim800l_config_t* config;
    TaskHandle_t sim800l_task_handle;
    EventGroupHandle_t sim800l_event_group_handle;
    esp_event_loop_handle_t sim800l_event_loop_handle;
    QueueHandle_t sim800l_queue_tx_handle;
    QueueHandle_t sim800l_queue_rx_handle;
};

/*
 *     event struct
 */
typedef struct sim800l_event_hash_t {
    const char *event_name;
    sim800l_event_t (*sim800l_event_callback)(char **input_args, void *output_data);
    struct sim800l_event_hash_t *chain;
} sim800l_event_hash_t;

/*
 *     event table
 */
static sim800l_event_hash_t *sim800l_event_table[TABLE_SIZE] = {NULL};

/*
 *     Private functions
 */
static esp_err_t sim800l_uart_init(sim800l_handle_t sim800l_handle);
static uint32_t sim800l_uart_send_data(sim800l_handle_t sim800l_handle, uint8_t* data, uint32_t data_len);
static uint32_t sim800l_uart_recv_data(sim800l_handle_t sim800l_handle, uint8_t* data, uint32_t data_len, uint32_t timeout);
static esp_err_t sim800l_post_event(sim800l_handle_t sim800l_handle, sim800l_event_t sim800l_event, void* data);
static uint32_t event_hash(const char *event_name);
static sim800l_event_t sim800l_event_interpreter(sim800l_handle_t sim800l_handle, const char *event_name, char *event_args[]);

/*
 *     SIM800L task
 */
static void sim800l_bridge_task(void* args);

/*
 *     SIM800L core callback functions
 */
sim800l_event_t sim800l_event_rdy(char **input_args, void *output_data);
sim800l_event_t sim800l_event_cfun(char **input_args, void *output_data);
sim800l_event_t sim800l_event_cpin(char **input_args, void *output_data);
sim800l_event_t sim800l_event_call_ready(char **input_args, void *output_data);
sim800l_event_t sim800l_event_sms_ready(char **input_args, void *output_data);

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

    /* Configure reset pin */
    ret = gpio_set_direction(sim800l_handle_temp->config->sim800l_rst_pin, GPIO_MODE_OUTPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "gpio_set_direction failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Setting power pin and reset pin */
    if (sim800l_handle_temp->config->sim800l_pwr_pin == GPIO_NUM_NC)
    {
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
    if (sim800l_handle_temp->config->sim800l_dtr_pin != GPIO_NUM_NC)
    {
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
    }

    /* Setting RING pin */
    if (sim800l_handle_temp->config->sim800l_ring_pin != GPIO_NUM_NC)
    {
        ret = gpio_set_direction(sim800l_handle_temp->config->sim800l_ring_pin, GPIO_MODE_INPUT);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_direction failed: %s", esp_err_to_name(ret));
            return ret;
        }
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
    sim800l_handle_temp->sim800l_queue_tx_handle = xQueueCreate(NUM_OF_PARAMS, MAX_PARAMS_SIZE);
    if (sim800l_handle_temp->sim800l_queue_tx_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "xQueueCreate failed");
        return ESP_ERR_NO_MEM;
    }

     /* Create Queue */
    sim800l_handle_temp->sim800l_queue_rx_handle = xQueueCreate(NUM_OF_PARAMS, MAX_PARAMS_SIZE);
    if (sim800l_handle_temp->sim800l_queue_rx_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "xQueueCreate failed");
        return ESP_ERR_NO_MEM;
    }

    /* Assign temporary handle to main handle */
    *sim800l_handle = sim800l_handle_temp;

    return ESP_OK;
}

esp_err_t sim800l_deinit(sim800l_handle_t sim800l_handle)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    /* Check if handle is NULL */
    if (sim800l_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /* Reset RST pin */
    esp_err_t ret = gpio_reset_pin(sim800l_handle->config->sim800l_rst_pin);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "gpio_reset_pin failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Reset PWR pin */
    if (sim800l_handle->config->sim800l_pwr_pin != GPIO_NUM_NC)
    {
        /* Reset PWRKEY pin */
        ret = gpio_reset_pin(sim800l_handle->config->sim800l_pwr_pin);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_reset_pin failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    /* Reset DTR pin */
    if (sim800l_handle->config->sim800l_dtr_pin != GPIO_NUM_NC)
    {
        /* Reset DTR pin */
        ret = gpio_reset_pin(sim800l_handle->config->sim800l_dtr_pin);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_reset_pin failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    /* Reset RING pin */
    if (sim800l_handle->config->sim800l_ring_pin != GPIO_NUM_NC)
    {
        /* Reset RING pin */
        ret = gpio_reset_pin(sim800l_handle->config->sim800l_ring_pin);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_reset_pin failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    /* Delete event loop */
    ret = esp_event_loop_delete(sim800l_handle->sim800l_event_loop_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "esp_event_loop_delete failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Delete Event Group */
    vEventGroupDelete(sim800l_handle->sim800l_event_group_handle);

    /* Delete Queue */
    vQueueDelete(sim800l_handle->sim800l_queue_tx_handle);
    vQueueDelete(sim800l_handle->sim800l_queue_rx_handle);

    free(sim800l_handle);
    sim800l_handle = NULL;

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
    if (xTaskCreate(sim800l_bridge_task,
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

    /* Register callback */
    ret = sim800l_register_callback(SIM800L_EVENT_RDY_STR, sim800l_event_rdy);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = sim800l_register_callback(SIM800L_EVENT_CFUN_STR, sim800l_event_cfun);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = sim800l_register_callback(SIM800L_EVENT_CPIN_STR, sim800l_event_cpin);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = sim800l_register_callback(SIM800L_EVENT_CALL_READY_STR, sim800l_event_call_ready);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = sim800l_register_callback(SIM800L_EVENT_SMS_READY_STR, sim800l_event_sms_ready);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* AT command test */
    if (sim800l_command_AT(sim800l_handle) != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_out_data failed");
        return ESP_FAIL;
    }

    /* Wait for SIM800L to be ready */
    BaseType_t events_start = xEventGroupWaitBits (sim800l_handle->sim800l_event_group_handle, 
                                                    SIM800L_EVENT_RDY |
                                                    SIM800L_EVENT_CFUN_FULL |
                                                    SIM800L_EVENT_CPIN_READY |
                                                    SIM800L_EVENT_CALL_READY |
                                                    SIM800L_EVENT_SMS_READY,
                                                    pdTRUE, 
                                                    pdTRUE, 
                                                    60000/portTICK_PERIOD_MS);
    if(events_start == (SIM800L_EVENT_RDY |
                        SIM800L_EVENT_CFUN_FULL |
                        SIM800L_EVENT_CPIN_READY |
                        SIM800L_EVENT_CALL_READY |
                        SIM800L_EVENT_SMS_READY ))
    {

        ESP_LOGI(SIM800L_TAG, "SIM800L is ready");
        ret = ESP_OK;
    }
    else
    {
        ESP_LOGE(SIM800L_TAG, "SIM800L is not ready");
        ret = ESP_FAIL;
    }

    /* Unregister callback */
    ret = sim800l_unregister_callback(SIM800L_EVENT_RDY_STR);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = sim800l_unregister_callback(SIM800L_EVENT_CFUN_STR);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = sim800l_unregister_callback(SIM800L_EVENT_CPIN_STR);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = sim800l_unregister_callback(SIM800L_EVENT_CALL_READY_STR);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = sim800l_unregister_callback(SIM800L_EVENT_SMS_READY_STR);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_register_callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ret;
}

esp_err_t sim800l_stop(sim800l_handle_t sim800l_handle)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    /* Check if handle is NULL */
    if (sim800l_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /* Stop task */
    vTaskDelete(sim800l_handle->sim800l_task_handle);

    esp_err_t ret = ESP_FAIL;

    /* Check PWRKEY */
    if (sim800l_handle->config->sim800l_pwr_pin == GPIO_NUM_NC)
    {
        /* Module control by reset pin */
        ret = gpio_set_level(sim800l_handle->config->sim800l_rst_pin, 0);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_level failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }   
    else
    {
        /* Turn on module */
        ret = gpio_set_level(sim800l_handle->config->sim800l_pwr_pin, 1);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SIM800L_TAG, "gpio_set_level failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    return ESP_OK;
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

            return ESP_FAIL;
        }
    }

    /* Check if response is NULL */
    if (response != NULL)
    {
        /* Send Queue */
        if (xQueueSend(sim800l_handle->sim800l_queue_tx_handle, command, timeout) != pdPASS)
        {
            ESP_LOGE(SIM800L_TAG, "xQueueSend failed"); 
            return ESP_FAIL;
        }

        char respo_temp[MAX_PARAMS_SIZE] = {0};

        /* Wait for response */
        if (xQueueReceive(sim800l_handle->sim800l_queue_rx_handle, respo_temp, timeout) != pdPASS)
        {
            ESP_LOGE(SIM800L_TAG, "xQueueReceive failed"); 
            return ESP_FAIL;
        }

        /* Copy response */
        memcpy(response, respo_temp, strlen(respo_temp));

        return ESP_OK;
    }

    return ESP_OK;
}

esp_err_t sim800l_out_data_event(sim800l_handle_t sim800l_handle, uint8_t *command, EventBits_t event, uint32_t timeout)
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

            return ESP_FAIL;
        }
    }

    /* Wait for response */
    BaseType_t events_ret = xEventGroupWaitBits(sim800l_handle->sim800l_event_group_handle, event, pdTRUE, pdTRUE, timeout);
    if (events_ret == event)
    {
        return ESP_OK;
    }

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
    ret = esp_event_handler_register_with(sim800l_handle->sim800l_event_loop_handle,
                                          SIM800L_EVENTS,
                                          (int32_t)sim800l_event,
                                          sim800l_event_handler,
                                          sim800l_event_handler_arg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "esp_event_handler_register failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ret;
}

esp_err_t sim800l_unregister_event(sim800l_handle_t sim800l_handle, sim800l_event_t sim800l_event, esp_event_handler_t sim800l_event_handler)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    esp_err_t ret = ESP_OK;

    /* Check if handle is NULL */
    if (sim800l_handle == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "sim800l_handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /* Unregister event handler */
    ret = esp_event_handler_unregister_with(sim800l_handle->sim800l_event_loop_handle,
                                            SIM800L_EVENTS,
                                            (int32_t)sim800l_event,
                                            sim800l_event_handler);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SIM800L_TAG, "esp_event_handler_unregister failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t sim800l_register_callback(const char *event_name, sim800l_event_t (*sim800l_event_callback)(char **input_args, void *output_data))
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    /* Get event hash */
    uint32_t index = event_hash(event_name);

    /* Create new event struct */
    sim800l_event_hash_t *new_event = calloc(1, sizeof(sim800l_event_hash_t));
    if (new_event == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "Memory allocation failed");
        return ESP_ERR_NO_MEM;
    }

    /* Assembly of the new event struct */
    new_event->event_name = strdup(event_name);
    if (new_event->event_name == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "Memory allocation failed");
        free(new_event);
        return ESP_ERR_NO_MEM;
    }

    /* Assembly of the new event struct */
    new_event->sim800l_event_callback = sim800l_event_callback;

    /* Check if event is already registered */
    if (sim800l_event_table[index] != NULL)
    {   
        /* Set chain of previous event */
        new_event->chain = sim800l_event_table[index];
    }

    /* Set new event struct */
    sim800l_event_table[index] = new_event;

    return ESP_OK;
}

esp_err_t sim800l_unregister_callback(const char *event_name)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    /* Get event hash */
    uint32_t index = event_hash(event_name);

    /* Get event struct */
    sim800l_event_hash_t *event = sim800l_event_table[index];
    sim800l_event_hash_t *prev_event = NULL;

    while (event)
    {
        /*Check if event name is equal to event_name*/
        if (strstr(event->event_name, event_name) != NULL)
        {   
            /* Check if chain is NULL */
            if (event->chain != NULL)
            {   
                /* Set chain of previous event */
                sim800l_event_table[index] = event->chain;
            }
            else
            {
                /* Clear table */
                sim800l_event_table[index] = NULL;
            }
            
            /* Free event struct */
            free(event);
            event = NULL;

            /* Clear chain of previous event */
            if (prev_event != NULL)
            {
                prev_event->chain = NULL;
            }

            return ESP_OK;
        }

        /* Set previous event */
        prev_event = event;

        /* Set next event */
        event = event->chain;
    }

    return ESP_OK;
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
    uart_config_t uart_config = {
        .baud_rate = sim800l_handle->config->sim800l_uart_baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    /* Config sim800l uart driver */
    ret = uart_param_config (sim800l_handle->config->sim800l_uart_port, &uart_config);
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
    ret = esp_event_post_to(sim800l_handle->sim800l_event_loop_handle, /* Event loop handle */
                            SIM800L_EVENTS,                            /* Events base */
                            sim800l_event,                             /* Event */
                            &sim800l_event_data,                       /* Event data */
                            sizeof(sim800l_event_data_t),                /* Event data size */
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

static sim800l_event_t sim800l_event_interpreter(sim800l_handle_t sim800l_handle, const char *event_name, char *event_args[])
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    /* Check if event_name is NULL */
    if (event_name == NULL)
    {
        ESP_LOGE(SIM800L_TAG, "event_name is NULL");
        return -1;
    }

    /* Get event hash */
    uint32_t hash = event_hash(event_name);

    /* Get event struct related to hash */
    sim800l_event_hash_t *event = sim800l_event_table[hash];

    while (event)
    {
        /*Check if event name is equal to event_name*/
        if (strstr(event->event_name, event_name) != NULL)
        {
            /* event callback */
            if (event->sim800l_event_callback != NULL)
            {   
                void *output = {0};

                sim800l_event_t ret = event->sim800l_event_callback((char**)event_args, &output);

                ESP_LOGI(SIM800L_TAG, "SIM800L EVENT: %s | ret: %d", event_name, ret);

                /* Set eventgroup */
                xEventGroupSetBits(sim800l_handle->sim800l_event_group_handle, ret);

                /* Post event with args */
                if (sim800l_post_event(sim800l_handle, ret, &output) != ESP_OK)
                {
                    ESP_LOGE(SIM800L_TAG, "sim800l_post_event failed");
                    return -1;
                }

                return ret;
            }
        }

        /* Get next event struct */
        event = event->chain;
    }

    return -1;
}

static uint32_t event_hash(const char *event_name)
{
    uint32_t event_hash = 10037;
    while (*event_name) {
        event_hash = (event_hash*31) + *event_name++;
    }
    return event_hash%TABLE_SIZE;
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
        uint8_t command_response[MAX_PARAMS_SIZE] = {0};
        uint8_t response[MAX_PARAMS_SIZE] = {0};

        /* Read data from sim800l uart */
        uint8_t data[MAX_PARAMS_SIZE] = {0};
        if (sim800l_uart_recv_data(sim800l_handle, data, sizeof(data), MAX_PARAMS_SIZE) > 0)
        {
            ESP_LOGI(SIM800L_TAG, "Received data: %s", data);

            /* Receive Queue */
            if (xQueueReceive(sim800l_handle->sim800l_queue_tx_handle, command_response, 0) == pdTRUE)
            {
                // ESP_LOGI(SIM800L_TAG, "Received command: %s", command_response);

                /* Data temp */
                uint8_t data_temp[MAX_PARAMS_SIZE] = {0};
                strncpy((char *)data_temp, (const char *)data, MAX_PARAMS_SIZE);

                /* Token response */
                char *token_respose = strtok((char *)data_temp, "\r\n");
                
                /* Check if the token is a command*/
                if (strnstr((const char *)command_response, (const char *)token_respose, strlen((const char *)command_response)) != NULL)
                {
                    /* Extract response */
                    token_respose = strtok(NULL, "\r\n");
                    do
                    {
                        // ESP_LOGI(SIM800L_TAG, "Token response: %s", token_respose);
                        /* Check if token starts with '+' */
                        if (token_respose[0] == '+')
                        {
                            /* Remove '+<>:' */
                            char *result_find = strchr((const char *)token_respose, ':');
                            if (result_find != NULL)
                            {
                                token_respose = result_find + 1;
                            }
                        }

                        /* Add token to response */
                        if (strncat((char *)response, (const char *)token_respose, strlen((const char *)token_respose)) == NULL)
                        {
                            ESP_LOGE(SIM800L_TAG, "strncat failed");
                        }

                        /* Add '\r\n' to response */
                        if (strncat((char *)response, "\r\n", strlen("\r\n") + 1) == NULL)
                        {
                            ESP_LOGE(SIM800L_TAG, "strncat failed");
                        }

                        /* Get next token */
                        token_respose = strtok(NULL, "\r\n");
                    } while (token_respose != NULL);
                    
                    /* Send response */
                    if (xQueueSend(sim800l_handle->sim800l_queue_rx_handle, response, 0) != pdPASS)
                    {
                        ESP_LOGE(SIM800L_TAG, "xQueueSend failed");
                    }
                }
            }

            /* Extract tokens */
            char **event_args = (char **)calloc(5, sizeof(char *));
            char *token = strtok((char*)data, "\r\n");
            do
            {
                // ESP_LOGI(SIM800L_TAG, "Token event: %s", token);
                memset(event_args, 0, sizeof(char *) * 5);
                char event[25] = {0};

                /* Check if the token is a event in format: +<token>:*/
                if (token[0] == '+')
                {
                    /* Extract event (+<event>:args) */
                    token = strtok(token, ":");
                    strncpy(event, token, strlen(token));
                    // ESP_LOGI(SIM800L_TAG, "Event: %s", event);

                    int i = 0;
                    token = strtok(NULL, ",");
                    do
                    {
                        /* Extract args */
                        event_args[i] = strdup(token);
                        
                        /* Get next token */
                        token = strtok(NULL, ",");
                        i++;
                    } while ((token != NULL) && (i < 5));
                }
                else
                {
                    /* Extract simple event */
                    strncpy(event, token, strlen(token));
                    // ESP_LOGI(SIM800L_TAG, "Event: %s", event);
                }

                /* Interpret event */
                sim800l_event_interpreter(sim800l_handle, (const char *)event, event_args);
                
                /* Get next token */
                token = strtok(NULL, "\r\n");
            } while (token != NULL);

            
            free(event_args);
            event_args = NULL;
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

/*
 *     SIM800L core callback functions 
 */
sim800l_event_t sim800l_event_rdy(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    return SIM800L_EVENT_RDY;
}

sim800l_event_t sim800l_event_cfun(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    if (strstr(input_args[0], "0") != NULL)
    {
        return SIM800L_EVENT_CFUN_MINIMUM;
    }
    if (strstr(input_args[0], "1") != NULL)
    {
        return SIM800L_EVENT_CFUN_FULL;
    }

    return -1;
}

sim800l_event_t sim800l_event_cpin(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    if (strstr(input_args[0], "READY") != NULL)
    {
        return SIM800L_EVENT_CPIN_READY;
    }

    return -1;
}

sim800l_event_t sim800l_event_call_ready(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    return SIM800L_EVENT_CALL_READY;
}

sim800l_event_t sim800l_event_sms_ready(char **input_args, void *output_data)
{
    ESP_LOGD(SIM800L_TAG, "%s", __func__);

    return SIM800L_EVENT_SMS_READY;
}