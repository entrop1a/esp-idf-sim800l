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
esp_err_t sim800l_command_AT(sim800l_handle_t sim800l_handle);
esp_err_t sim800l_command_set_baud_rate(sim800l_handle_t sim800l_handle, uint32_t baud_rate);

#ifdef __cplusplus
}
#endif