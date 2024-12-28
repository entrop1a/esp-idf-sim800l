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
    SIM800L_SMS_MODE_PDU = 0,
    SIM800L_SMS_MODE_TEXT
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

/*
 *     SIM800L SMS
 */
typedef struct {
    char source_number[16];
    char utc_date[32];
    char payload[160];
} sim800l_sms_message_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 *     SIM800L functions prototypes
 */
sim800l_ret_t sim800l_sms_switch(sim800l_handle_t sim800l_handle, bool enable);
sim800l_ret_t sim800l_sms_set_mode(sim800l_handle_t sim800l_handle, sim800l_sms_mode_t sms_mode);
sim800l_sms_mode_t sim800l_sms_get_mode(sim800l_handle_t sim800l_handle);
sim800l_ret_t sim800l_sms_read_message(sim800l_handle_t sim800l_handle, uint32_t index, sim800l_sms_message_t *message);
sim800l_ret_t sim800l_sms_send_message(sim800l_handle_t sim800l_handle, const char *number, const char *message);
sim800l_ret_t sim800l_sms_delete_message(sim800l_handle_t sim800l_handle, uint32_t delete_flag, uint32_t index);

#ifdef __cplusplus
}
#endif