#include <stdio.h>
#include <driver/gpio.h>
#include "sim800l_core.h"
#include "sim800l_misc.h"
#include "sim800l_call.h"

/* Tag */
#define TAG_SIM800L_EXAMPLE "SIM800L EXAMPLE"

bool _flag = false;

/* SIM800L handle */
sim800l_handle_t sim800l_handle;

/* SIM800L config struct */
sim800l_config_t sim800l_config = {
    .sim800l_uart_port = UART_NUM_1,
    .sim800l_uart_baudrate = 9600,
    .sim800l_uart_rx_pin = GPIO_NUM_27,
    .sim800l_uart_tx_pin = GPIO_NUM_26,
    .sim800l_rst_pin = GPIO_NUM_25,
    .sim800l_pwr_pin = GPIO_NUM_NC,
    .sim800l_dtr_pin = GPIO_NUM_32,
    .sim800l_ring_pin = GPIO_NUM_33};

static void sim800l_event_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data)
{
    sim800l_event_data_t *data = (sim800l_event_data_t *)event_data;

    switch (event_id)
    {
    case SIM800L_EVENT_CALL_RING:
    {
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L EVENT CALL RING");

        _flag = true;

        break;
    }
    case SIM800L_EVENT_CALL_IDENTIFY:
    {
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L EVENT CALL IDENTIFY");

        sim800l_call_identify_t *call_identify = data->ptr;

        ESP_LOGI(TAG_SIM800L_EXAMPLE, "Identify Number: %s", call_identify->number);
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "Identify Type: %lu", call_identify->type);

        break;
    }
    case SIM800L_EVENT_CALL_NO_CARRIER:
    {
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L EVENT CALL NO CARRIER");

        _flag = false; 

        break;
    }
    default:
        break;
    }
}

void app_main(void)
{
    esp_err_t ret = ESP_OK;

    /* Init SIM800L */
    ret = sim800l_init(&sim800l_handle, &sim800l_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L init failed: %s", esp_err_to_name(ret));
        ESP_ERROR_CHECK(ret);
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L init success");

    /* Start SIM800L task */
    ret = sim800l_start(sim800l_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L start failed: %s", esp_err_to_name(ret));
        ESP_ERROR_CHECK(ret);
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L start success");

    /* Register SIM800L event */
    ret = sim800l_register_event(sim800l_handle, SIM800L_EVENT_ANY_ID, sim800l_event_handler, NULL);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L register event failed: %s", esp_err_to_name(ret));
        ESP_ERROR_CHECK(ret);
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L register event success");

    /* SIM800L call enable or disable call events */
    if (sim800l_call_switch(sim800l_handle, true) != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L call switch failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L call switch success");

    /* SIM800L call line identify */
    if (sim800l_call_line_identify(sim800l_handle, true) != SIM800L_RET_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L call line identify failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L call line identify success");

    /* Wait */
    while (!_flag)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    /* SIM800L call answer */
    if (sim800l_call_response(sim800l_handle, SIM800L_CALL_ANSWER) != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L call answer failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L call answer success");

    /* Wait */
    while (_flag)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG_SIM800L_EXAMPLE, "Finish example");    
}