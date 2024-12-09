/*
 * @file sim800l_sms.h
 * @author Eduardo Gomes
 * @brief Header file for SIM800L SMS functions
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
#include "sim800l_core.h"
#include "sim800l_common.h"
#include <esp_err.h>

/*
 *     SIM800L SMS mode
 */
typedef enum {
    SIM800L_SMS_MODE_TEXT = 0,
    SIM800L_SMS_MODE_PDU
} sim800l_sms_mode_t;

/*
 *     SIM800L SMS list type
 */
typedef enum {
    SIM800L_SMS_UNREAD_MESSAGES = 0,
    SIM800L_SMS_READ_MESSAGES,
    SIM800L_SMS_UNSENT_MESSAGES,
    SIM800L_SMS_SENT_MESSAGES,
    SIM800L_SMS_ALL_MESSAGES
} sim800l_sms_list_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 *     SIM800L functions prototypes
 */
esp_err_t sim800l_sms_sel_mode(sim800l_handle_t sim800l_handle, sim800l_sms_mode_t sms_mode);
esp_err_t sim800l_sms_list_messages(sim800l_handle_t sim800l_handle, sim800l_sms_list_t sms_list);
esp_err_t sim800l_sms_read_message(sim800l_handle_t sim800l_handle, uint32_t index, uint8_t *message);
esp_err_t sim800l_sms_send_message(sim800l_handle_t sim800l_handle, uint8_t *number, uint8_t *message);
esp_err_t sim800l_sms_delete_message(sim800l_handle_t sim800l_handle, uint32_t delete_flag, uint32_t index);
esp_err_t sim800l_sms_store_message(sim800l_handle_t sim800l_handle, uint8_t *message);
esp_err_t sim800l_sms_send_stored_message(sim800l_handle_t sim800l_handle, uint32_t index, uint8_t *message);

#ifdef __cplusplus
}
#endif