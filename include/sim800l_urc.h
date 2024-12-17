/*
 * @file sim800l_urc.h
 * @author Eduardo Gomes
 * @brief Header file for SIM800L general functions
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

#ifdef __cplusplus
extern "C" {
#endif

/*
 *     SIM800L URC events
 */
typedef enum 
{
    SIM800L_URC_EVENT_RDY           = 1 << 0,
    SIM800L_URC_EVENT_CFUN_MINIMUM  = 1 << 1,
    SIM800L_URC_EVENT_CFUN_FULL     = 1 << 2,
    SIM800L_URC_EVENT_CFUN_DISABLE  = 1 << 3,
    SIM800L_URC_EVENT_CPIN_READY    = 1 << 4,
    SIM800L_URC_EVENT_CALL_READY    = 1 << 5,
    SIM800L_URC_EVENT_SMS_READY     = 1 << 6,
    SIM800L_URC_EVENT_SMS_SEND      = 1 << 7
}
sim800l_urc_event_t;

/*
 *     SIM800L functions prototypes
 */
bool sim800l_urc_create_table(void);
sim800l_urc_event_t sim800l_urc_interpret(const char *urc_name, char *urc_args[]);

#ifdef __cplusplus
}
#endif