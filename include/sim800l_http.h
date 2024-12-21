/*
 * @file sim800l_http.h
 * @author Eduardo Gomes
 * @brief Header file for SIM800L HTTP functions
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
 *     SIM800L HTTP param
 */
typedef enum 
{
    SIM800L_HTTP_PARAM_CID = 0,
    SIM800L_HTTP_PARAM_URL,
    SIM800L_HTTP_PARAM_UA,
    SIM800L_HTTP_PARAM_PROIP,
    SIM800L_HTTP_PARAM_PROPORT,
    SIM800L_HTTP_PARAM_REDIR,
    SIM800L_HTTP_PARAM_BREAK,
    SIM800L_HTTP_PARAM_BREAKEND,
    SIM800L_HTTP_PARAM_TIMEOUT,
    SIM800L_HTTP_PARAM_CONTENT,
    SIM800L_HTTP_PARAM_USERDATA
} sim800l_http_param_tag_t;

typedef struct 
{
    uint32_t cid;
    char *url;
    char *ua;
    char *proip;
    char *proport;
    uint32_t *redir;
    char *break_;
    char *breakend;
    uint32_t *timeout;
    char *content;
    char *userdata;
} sim800l_http_param_t;

typedef enum
{
    SIM800L_HTTP_METHOD_GET = 0,
    SIM800L_HTTP_METHOD_POST,
    SIM800L_HTTP_METHOD_HEAD,
    SIM800L_HTTP_METHOD_DELETE
}sim800l_http_method_t;

typedef struct
{
    sim800l_http_method_t method;
    uint32_t http_code;
    uint32_t content_length;
}sim800l_http_action_t;


/* 
 *     Functions 
 */
sim800l_ret_t sim800l_http_switch(sim800l_handle_t sim800l_handle, bool enable);
sim800l_ret_t sim800l_http_set_param(sim800l_handle_t sim800l_handle, sim800l_http_param_tag_t param_tag, const char *value);
sim800l_ret_t sim800l_http_get_param(sim800l_handle_t sim800l_handle, sim800l_http_param_t *param);
sim800l_ret_t sim800l_http_action(sim800l_handle_t sim800l_handle, sim800l_http_method_t method, sim800l_http_action_t *ret_action);
sim800l_ret_t sim800l_http_read(sim800l_handle_t sim800l_handle, uint32_t start_addr, size_t length, uint8_t *buffer);
