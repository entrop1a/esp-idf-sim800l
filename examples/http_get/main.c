#include <stdio.h>
#include <string.h>
#include <driver/gpio.h>
#include "sim800l_core.h"
#include "sim800l_misc.h"
#include "sim800l_bearer.h"
#include "sim800l_http.h"

/* Tag */
#define TAG_SIM800L_EXAMPLE "SIM800L HTTP GET EXAMPLE"

bool _flag = false;
sim800l_http_action_t action;

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

static void sim800l_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
{
    sim800l_event_data_t *data = (sim800l_event_data_t *)event_data;

    switch (event_id)
    {
    case SIM800L_EVENT_HTTP_ACTION:
    {
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L EVENT HTTP ACTION");

        memcpy(&action, data->ptr, sizeof(sim800l_http_action_t));

        ESP_LOGI(TAG_SIM800L_EXAMPLE, "Method: %d", action.method);
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "HTTP code: %lu", action.http_code);
        ESP_LOGI(TAG_SIM800L_EXAMPLE, "Content-length: %lu", action.content_length);

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

    /* Set Bearer contype */
    if (sim800l_bearer_set_param(sim800l_handle, SIM800L_BEARER_CONTYPE, "GPRS") != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L set bearer contype failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L set bearer contype success");

    /* Set Bearer APN */
    if (sim800l_bearer_set_param(sim800l_handle, SIM800L_BEARER_APN, "timbrasil.br") != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L set bearer APN failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L set bearer APN success");

    /* Set Bearer USER */
    if (sim800l_bearer_set_param(sim800l_handle, SIM800L_BEARER_USER, "tim") != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L set bearer USER failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L set bearer USER success");

    /* Set Bearer PASS */
    if (sim800l_bearer_set_param(sim800l_handle, SIM800L_BEARER_PWD, "tim") != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L set bearer PASS failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L set bearer PASS success");

    /* Enable bearer */
    if (sim800l_bearer_switch(sim800l_handle, true) != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L enable bearer failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L enable bearer success");

    /* Init HTTP */
    if (sim800l_http_switch(sim800l_handle, true) != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L init HTTP failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L init HTTP success");

    /* Set CID */
    if (sim800l_http_set_param(sim800l_handle, SIM800L_HTTP_PARAM_CID, "1") != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L set CID failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L set CID success");

    /* Set URL */
    if (sim800l_http_set_param(sim800l_handle, SIM800L_HTTP_PARAM_URL, "www.helloworld.org/data/helloworld.c") != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L set URL failed");
        return;
    }
    ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L set URL success");

    /* HTTP action */
    if (sim800l_http_action(sim800l_handle, SIM800L_HTTP_METHOD_GET) != ESP_OK)
    {
        ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L HTTP action failed");
        return;
    }

    while (true)
    {
        if (_flag)
        {
            uint32_t data_len = action.content_length;

            ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L HTTP content-length: %lu", data_len);

            char buffer[100] = {0};

            /* Read */
            if (sim800l_http_read(sim800l_handle, 0, data_len, (uint8_t *)buffer) != ESP_OK)
            {
                ESP_LOGE(TAG_SIM800L_EXAMPLE, "SIM800L HTTP read failed");
                return;
            }
            ESP_LOGI(TAG_SIM800L_EXAMPLE, "SIM800L HTTP read success: %s", buffer);

            _flag = false;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG_SIM800L_EXAMPLE, "Finish example");
}