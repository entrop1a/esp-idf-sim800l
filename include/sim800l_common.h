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

/*
 *     URC names
 */

#define SIM800L_URC_RDY "RDY"
#define SIM800L_URC_CFUN "+CFUN"
#define SIM800L_URC_CPIN "+CPIN"
#define SIM800L_URC_CALL_READY "Call Ready"
#define SIM800L_URC_SMS_READY "SMS Ready"

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
#define SIM800L_COMMAND_LINE_ID "AT+CLIP="

/*
 * SIM800L - SMS mode.
 *
 * This command is used to set the SMS mode.
 * 
 */
#define SIM800L_COMMAND_SMS_MODE "AT+CMGF="

/*
 * SIM800L - SMS list received Message.
 *
 * This command is used to list the received messages.
 * 
 */
#define SIM800l_COMMAND_SMS_LIST_RECV "AT+CMGL="