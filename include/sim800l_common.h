/*
 * @file sim800l_commands.h
 * @author Eduardo Gomes
 * @brief Header file for SIM800L commands definitions
 * 
 * @copyright MIT
 * 
 */

/* 
 *     Preprocessor guard 
 */
#pragma once

/*
 *     SIM800L returns codes
 */

typedef int sim800l_ret_t;

#define SIM800L_RET_OK 0
#define SIM800L_RET_ERROR -1
#define SIM800L_RET_ERROR_MEM 1
#define SIM800L_RET_ERROR_BUILD_COMMAND 2
#define SIM800L_RET_ERROR_SEND_COMMAND 3
#define SIM800L_RET_INVALID_ARG 4

/*
 *     EVENT names
 */

#define SIM800L_EVENT_RDY_STR "RDY"
#define SIM800L_EVENT_CFUN_STR "+CFUN"
#define SIM800L_EVENT_CPIN_STR "+CPIN"
#define SIM800L_EVENT_CALL_READY_STR "Call Ready"
#define SIM800L_EVENT_SMS_READY_STR "SMS Ready"

/*
 * SIM800L - Test command
 *
 * This command is used to test the SIM800L module.
 *
 */
#define SIM800L_COMMAND_AT "AT\r\n"

/*
 * SIM800L - Power Off command
 *
 * This command is used to turn off the SIM800L module.
 *
 */
#define SIM800L_COMMAND_PWR_OFF "AT+CPOWD="


/*
 * SIM800L - Set command echo mode.
 *
 * This command is used to enable or disable the echo mode.
 *
 */
#define SIM800L_COMMAND_ECHO_MODE "ATE"

/*
 * SIM800L - Set baud rate.
 *
 * This command is used to set the baud rate.
 *
 */
#define SIM800L_COMMAND_SET_BAUD_RATE "AT+IPR="

/*
 * SIM800L - Save parameters in NVRAM.
 *
 * This command is used to save parameters in NVRAM.
 * 
 */
#define SIM800L_COMMAND_SAVE_PARAMS "AT&W_SAVE\r\n"

/*
 * SIM800L - Originate call.
 *
 * This command is used to originate a call.
 * 
 */
#define SIM800L_COMMAND_CALL "ATD+ "


/*
 * SIM800L - Answer call.
 *
 * This command is used to answer a call.
 * 
 */
#define SIM800L_COMMAND_CALL_ANSWER "ATA\r\n"


/*
 * SIM800L - End call.
 *
 * This command is used to end a call.
 * 
 */
#define SIM800L_COMMAND_CALL_HANGUP "ATH\r\n"


/*
 * SIM800L - Speaker volume.
 *
 * This command is used to set the speaker volume.
 * 
 */
#define SIM800L_COMMAND_SPEAKER_VOLUME "AT+CLVL="


/*
 * SIM800L - Speaker mute.
 *
 * This command is used to mute the speaker.
 * 
 */
#define SIM800L_COMMAND_MUTE_CALL "AT+CMUT="


/*
 * SIM800L - Line identification presentation.
 *
 * This command is used to set the line identification presentation.
 * 
 */
#define SIM800L_COMMAND_CALL_LINE_ID "AT+CLIP="

/*
 * SIM800L - SMS mode.
 *
 * This command is used to set the SMS mode.
 * 
 */
#define SIM800L_COMMAND_SMS_MODE "AT+CMGF"

/*
 * SIM800L - SMS list received Message.
 *
 * This command is used to list the received messages.
 * 
 */
#define SIM800l_COMMAND_SMS_LIST_RECV "AT+CMGL"


/*
 * SIM800L - SMS delete Message.
 *
 * This command is used to delete a message.
 * 
 */
#define SIM800L_COMMAND_SMS_DEL "AT+CMGD"

/*
 * SIM800L - SMS read Message.
 *
 * This command is used to read a message.
 *
 */
#define SIM800L_COMMAND_SMS_READ "AT+CMGR"


/*
 * SIM800L - SMS send Message.
 *
 * This command is used to send a message.
 *
 */
#define SIM800L_COMMAND_SMS_SEND "AT+CMGS"

/*
 * SIM800L - SMS bearer settings.
 *
 * This command is used to set the bearer settings.
 *
 */
#define SIM800L_COMMAND_BEARER "AT+SAPBR"

/*
 * SIM800L - HTTP init.
 *
 * This command is used to initialize the HTTP module.
 *
 */
#define SIM800L_COMMAND_HTTP_INIT "AT+HTTPINIT\r\n"

/*
 * SIM800L - HTTP terminate.
 *
 * This command is used to terminate the HTTP module.
 *
 */
#define SIM800L_COMMAND_HTTP_TERMINATE "AT+HTTPTERM\r\n"

/*
 * SIM800L - HTTP parameter.
 *
 * This command is used to set or get the HTTP parameter.
 *
 */
#define SIM800L_COMMAND_HTTP_PARAM "AT+HTTPPARA"

/*
 * SIM800L - HTTP action.
 *
 * This command is used to perform an HTTP action.
 *
 */
#define SIM800L_COMMAND_HTTP_ACTION "AT+HTTPACTION"

/*
 * SIM800L - HTTP read.
 *
 * This command is used to read data from the HTTP server.
 *
 */
#define SIM800L_COMMAND_HTTP_READ "AT+HTTPREAD"