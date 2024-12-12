/*
 * @file sim800l_general.h
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
 *     SIM800L functions prototypes
 */
sim800l_ret_t sim800l_command_AT(sim800l_handle_t sim800l_handle);

#ifdef __cplusplus
}
#endif