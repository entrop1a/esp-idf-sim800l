#include <stdio.h>
#include <driver/gpio.h>
#include "sim800l_core.h"
#include "sim800l_misc.h"
#include "sim800l_call.h"

/* Tag */
#define TAG_SIM800L_EXAMPLE "SIM800L MAKE CALL EXAMPLE"

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
    case SIM800L_EVENT_CALL_NO_CARRIER:
    {
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L EVENT CALL NO CARRIER");

        _flag = true; 

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

    /* SIM800L make call */
    if (sim800l_call_make_call(sim800l_handle, "+5581996348806") != SIM800L_RET_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L call make call failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L call make call success");

    while (_flag)
    {
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "Wait for call no carrier");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG_SIM800L_EXAMPLE, "Finish example");    
}