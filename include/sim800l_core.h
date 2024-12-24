/*
 * @file sim800l.h
 * @author Eduardo Gomes
 * @brief Header file for SIM800L driver
 * 
 * @copyright MIT
 *
 */

/* 
 *     Preprocessor guard 
 */
#pragma once

/* 
 *     Includes 
 */
#include <stdint.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_event.h>
#include <driver/uart.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *     SIM800L handle
 */
typedef struct sim800l* sim800l_handle_t;

/*
 *     SIM800L Config
 */
typedef struct
{
    uint32_t sim800l_uart_port;
    uint32_t sim800l_uart_baudrate;
    uint32_t sim800l_uart_rx_pin;
    uint32_t sim800l_uart_tx_pin;
    uint32_t sim800l_rst_pin;
    uint32_t sim800l_pwr_pin;
    uint32_t sim800l_dtr_pin;
    uint32_t sim800l_ring_pin;
} sim800l_config_t;


typedef enum 
{
    SIM800L_EVENT_ANY_ID        = ESP_EVENT_ANY_ID,
    SIM800L_EVENT_RDY           = 1 << 0,
    SIM800L_EVENT_CFUN_MINIMUM  = 1 << 1,
    SIM800L_EVENT_CFUN_FULL     = 1 << 2,
    SIM800L_EVENT_CFUN_DISABLE  = 1 << 3,
    SIM800L_EVENT_CPIN_READY    = 1 << 4,
    SIM800L_EVENT_CALL_READY    = 1 << 5,
    SIM800L_EVENT_SMS_READY     = 1 << 6,
    SIM800L_EVENT_SMS_SEND      = 1 << 7
}
sim800l_event_t;

/*
 *     Event base declaration
 */
ESP_EVENT_DECLARE_BASE(SIM800L_EVENTS);

typedef struct
{
    sim800l_handle_t sim800l_handle;
    void *ptr;
}sim800l_event_data_t;

/*
 *     SIM800L functions prototypes
 */

esp_err_t sim800l_init(sim800l_handle_t *sim800l_handle, sim800l_config_t *sim800l_config);
esp_err_t sim800l_deinit(sim800l_handle_t sim800l_handle);
esp_err_t sim800l_start(sim800l_handle_t sim800l_handle);
esp_err_t sim800l_stop(sim800l_handle_t sim800l_handle);
esp_err_t sim800l_out_data(sim800l_handle_t sim800l_handle, uint8_t *command, uint8_t *response, uint32_t timeout);
esp_err_t sim800l_out_data_event(sim800l_handle_t sim800l_handle, uint8_t *command, EventBits_t event, uint32_t timeout);
esp_err_t sim800l_register_event(sim800l_handle_t sim800l_handle, sim800l_event_t sim800l_event, esp_event_handler_t sim800l_event_handler, void *sim800l_event_handler_arg);
esp_err_t sim800l_unregister_event(sim800l_handle_t sim800l_handle, sim800l_event_t sim800l_event, esp_event_handler_t sim800l_event_handler);
esp_err_t sim800l_register_callback(const char *event_name, sim800l_event_t (*sim800l_event_callback)(char **args));
esp_err_t sim800l_unregister_callback(const char *event_name);


#ifdef __cplusplus
}
#endif
