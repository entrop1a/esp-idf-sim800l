/*
 * @file sim800l_call.h
 * @author Eduardo Gomes
 * @brief Header file for SIM800L call functions
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
 *     SIM800L call response type
 */
typedef enum {
    SIM800L_CALL_HANGUP = 0,
    SIM800L_CALL_ANSWER
} sim800l_call_response_t;

/*
 *     SIM800L call line identify
 */
typedef enum {
    SIM800L_CALL_LINE_ID_DIS = 0,
    SIM800L_CALL_LINE_ID_EN
} sim800l_call_line_id_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 *     SIM800L functions prototypes
 */
sim800l_ret_t sim800l_call_make_call(sim800l_handle_t sim800l_handle, const char *number);
sim800l_ret_t sim800l_call_response(sim800l_handle_t sim800l_handle, sim800l_call_response_t call_response);
sim800l_ret_t sim800l_call_line_identify(sim800l_handle_t sim800l_handle, sim800l_call_line_id_t call_line_id);

#ifdef __cplusplus
}
#endif