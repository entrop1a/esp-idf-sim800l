/*
 * @file sim800l_bearer.h
 * @author Eduardo Gomes
 * @brief Header file for SIM800L bearer functions
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
#include <sim800l_common.h>


/* 
 *     Sim800L bearer
 */
#define SIM800L_BEARER_CONTYPE "CONTYPE"
#define SIM800L_BEARER_APN "APN"
#define SIM800L_BEARER_USER "USER"
#define SIM800L_BEARER_PWD "PWD"
#define SIM800L_BEARER_PHONENUM "PHONENUM"
#define SIM800L_BEARER_RATE "RATE"

typedef struct
{
    uint32_t cid;
    uint32_t status;
    char ipv4[16];
}sim800l_bearer_t;

typedef struct
{
    char contype[5];
    char apn[64];
    char phonenum[16];
    char user[32];
    char pwd[32];
    uint32_t rate;
} sim800l_bearer_param_t;

sim800l_ret_t sim800l_bearer_switch(sim800l_handle_t sim800l_handle, bool bearer_state);
sim800l_ret_t sim800l_bearer_query(sim800l_handle_t sim800l_handle, sim800l_bearer_t *bearer);
sim800l_ret_t sim800l_bearer_set_param(sim800l_handle_t sim800l_handle, const char* param, const char* value);
sim800l_ret_t sim800l_bearer_get_param(sim800l_handle_t sim800l_handle, sim800l_bearer_param_t *param);
