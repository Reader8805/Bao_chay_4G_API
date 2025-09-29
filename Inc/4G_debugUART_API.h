#ifndef INC_4G_API_H_
#define INC_4G_API_H_

#include "stm32h7xx.h" // có thể đối sang dòng f1, f4,....
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#endif /* INC_4G_API_H_ */


typedef enum {
	SIM_RES_TIMEOUT_DB=0,
	SIM_RES_OK_DB,
	SIM_RES_CME_DB,
	SIM_RES_CMS_DB,
	SIM_RES_ERROR_DB
} sim_res_db_t;

sim_res_db_t sim_wait_result_db(UART_HandleTypeDef *uart_sim, uint32_t total_ms);
bool sim_wait_prompt_db(UART_HandleTypeDef *uart_sim, uint32_t max_ms);
void sim_cmd_expect_ok_db(UART_HandleTypeDef *uart_sim, UART_HandleTypeDef *uart_dbg, const char *cmd, uint32_t wait_ms);
void utf8ToUcs2Hex_db(const char* u8, char *out_hex, size_t out_sz);
void encodePhoneNumberPDU_db(const char* phone, char *out_sw, size_t out_sz, uint8_t *out_digits);
void buildPDU_db(const char* phone, const char* text, char *out_pdu, size_t out_sz);
void sim_init_db(UART_HandleTypeDef *uart_sim, UART_HandleTypeDef *uart_dbg);
void send_SMS_db(UART_HandleTypeDef *uart_sim, UART_HandleTypeDef *uart_dbg, const char *phone, const char *text);
