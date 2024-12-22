/*
 * @file sim800l_urc.c
 * @author Eduardo Gomes
 * @brief Source file for SIM800L general functions
 * 
 * @copyright MIT
 * 
 */

/*
 *     Includes
 */
#include "sim800l_urc.h"
#include "sim800l_core.h"
#include <esp_log.h>
#include <string.h>

/*
 *     Tag
 */
#define SIM800L_URC_TAG "SIM800L URC"

/*
 *     Define
 */
#define TABLE_SIZE 8

/*
 *     URC struct
 */
typedef struct sim800l_urc_t {
    const char *urc_name;
    uint32_t urc_value;
    sim800l_urc_event_t (*sim800l_urc_callback)(char **args);
    struct sim800l_urc_t *chain;
} sim800l_urc_t;

/*
 *     URC table
 */
static sim800l_urc_t *sim800l_urc_table[TABLE_SIZE] = {NULL};

/*
 *     Private functions prototypes
 */
static uint32_t urc_hash(const char *urc_name);
bool insert_urc_table(const char *urc_name, int value, sim800l_urc_event_t (*sim800l_urc_callback)(char **args));

/*
 *     Callback functions prototypes
 */
static sim800l_urc_event_t sim800l_urc_rdy(char **args);
static sim800l_urc_event_t sim800l_urc_cfun(char **args);
static sim800l_urc_event_t sim800l_urc_cpin(char **args);
static sim800l_urc_event_t sim800l_urc_call_ready(char **args);
static sim800l_urc_event_t sim800l_urc_sms_ready(char **args);
static sim800l_urc_event_t sim800l_urc_sms_send(char **args);

/*
 *     Public functions development
 */
bool sim800l_urc_create_table(void)
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    /* Insert URC RDY */
    if (insert_urc_table(SIM800L_URC_RDY, 1, sim800l_urc_rdy) == false)
    {
        ESP_LOGE(SIM800L_URC_TAG, "insert_urc_table failed");
        return false;
    }

    /* Insert URC CFUN */
    if (insert_urc_table(SIM800L_URC_CFUN, 2, sim800l_urc_cfun) == false)
    {
        ESP_LOGE(SIM800L_URC_TAG, "insert_urc_table failed");
        return false;
    }

    /* Insert URC CPIN */
    if (insert_urc_table(SIM800L_URC_CPIN, 3, sim800l_urc_cpin) == false)
    {
        ESP_LOGE(SIM800L_URC_TAG, "insert_urc_table failed");
        return false;
    }

    /* Insert URC CALL_READY */
    if (insert_urc_table(SIM800L_URC_CALL_READY, 4, sim800l_urc_call_ready) == false)
    {
        ESP_LOGE(SIM800L_URC_TAG, "insert_urc_table failed");
        return false;
    }

    /* Insert URC SMS_READY */
    if (insert_urc_table(SIM800L_URC_SMS_READY, 5, sim800l_urc_sms_ready) == false) 
    {
        ESP_LOGE(SIM800L_URC_TAG, "insert_urc_table failed");
        return false;
    }
    
    /* Insert URC SMS_SEND */
    if (insert_urc_table(SIM800L_URC_SMS_SEND, 6, sim800l_urc_sms_send) == false)
    {
        ESP_LOGE(SIM800L_URC_TAG, "insert_urc_table failed");
        return false;
    }

    return true;
}
   
void sim800l_urc_delete_table(void)
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        sim800l_urc_t *urc = sim800l_urc_table[i];
        while (urc)
        {
            sim800l_urc_t *aux = urc;
            urc = urc->chain;
            free(aux);
        }
        sim800l_urc_table[i] = NULL;
    }
}

sim800l_urc_event_t sim800l_urc_interpret(const char *urc_name, char *urc_args[])
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    /* Check if urc_name is NULL */
    if (urc_name == NULL)
    {
        ESP_LOGE(SIM800L_URC_TAG, "urc_name is NULL");
        return -1;
    }

    /* Get URC hash */
    uint32_t hash = urc_hash(urc_name);

    /* Get URC struct related to hash */
    sim800l_urc_t *urc = sim800l_urc_table[hash];

    while (urc)
    {
        /*Check if URC name is equal to urc_name*/
        if (strstr(urc->urc_name, urc_name) != NULL)
        {
            /* URC callback */
            if (urc->sim800l_urc_callback != NULL)
            {
                return urc->sim800l_urc_callback((char**)urc_args);
            }
        }

        urc = urc->chain;
    }

    return -1;
}

/*
 *     Private functions development
 */
static uint32_t urc_hash(const char *urc_name)
{
    uint32_t urc_hash = 10037;
    while (*urc_name) {
        urc_hash = (urc_hash*31) + *urc_name++;
    }
    return urc_hash%TABLE_SIZE;
}

bool insert_urc_table(const char *urc_name, int value, sim800l_urc_event_t (*sim800l_urc_callback)(char **args))
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);
    
    /* Get URC hash */
    uint32_t index = urc_hash(urc_name);

    /* Create new URC struct */
    sim800l_urc_t *new_urc = calloc(1, sizeof(sim800l_urc_t));
    if (new_urc == NULL)
    {
        ESP_LOGE(SIM800L_URC_TAG, "Memory allocation failed");
        return false;
    }

    /* Assembly of the new URC struct */
    new_urc->urc_name = strdup(urc_name);
    if (new_urc->urc_name == NULL)
    {
        ESP_LOGE(SIM800L_URC_TAG, "Memory allocation failed");
        free(new_urc);
        return false;
    }

    /* Assembly of the new URC struct */
    new_urc->urc_value = value;
    new_urc->sim800l_urc_callback = sim800l_urc_callback;
    new_urc->chain = sim800l_urc_table[index];
    sim800l_urc_table[index] = new_urc;

    return true;
}

/*
 *     Callback functions development
 */
sim800l_urc_event_t sim800l_urc_rdy(char **args)
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    return SIM800L_URC_EVENT_RDY;
}

sim800l_urc_event_t sim800l_urc_cfun(char **args)
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    if (strstr(args[0], "0") != NULL)
    {
        return SIM800L_URC_EVENT_CFUN_MINIMUM;
    }
    if (strstr(args[0], "1") != NULL)
    {
        return SIM800L_URC_EVENT_CFUN_FULL;
    }

    return -1;
}

sim800l_urc_event_t sim800l_urc_cpin(char **args)
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    if (strstr(args[0], "READY") != NULL)
    {
        return SIM800L_URC_EVENT_CPIN_READY;
    }

    return -1;
}

sim800l_urc_event_t sim800l_urc_call_ready(char **args)
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    return SIM800L_URC_EVENT_CALL_READY;
}

sim800l_urc_event_t sim800l_urc_sms_ready(char **args)
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    return SIM800L_URC_EVENT_SMS_READY;
}

sim800l_urc_event_t sim800l_urc_sms_send(char **args)
{
    ESP_LOGD(SIM800L_URC_TAG, "%s", __func__);

    return SIM800L_URC_EVENT_SMS_SEND;
}