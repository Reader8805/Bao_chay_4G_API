/*
 * 4G_API.h
 *
 *  Created on: Sep 29, 2025
 *      Author: DANG
 */

#ifndef INC_4G_API_H_
#define INC_4G_API_H_

#include "stm32h7xx.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#endif /* INC_4G_API_H_ */


typedef enum {
	SIM_RES_TIMEOUT=0,
	SIM_RES_OK,
	SIM_RES_CME,
	SIM_RES_CMS,
	SIM_RES_ERROR
} sim_res_t;

//sim_res_t sim_wait_result(UART_HandleTypeDef *uart_sim, uint32_t total_ms);
void sim_wait_result(UART_HandleTypeDef *uart_sim, uint32_t total_ms);
bool sim_wait_prompt(UART_HandleTypeDef *uart_sim, uint32_t max_ms);
void sim_cmd_expect_ok(UART_HandleTypeDef *uart_sim,/* UART_HandleTypeDef *uart_dbg,*/ const char *cmd, uint32_t wait_ms);
void utf8ToUcs2Hex(const char* u8, char *out_hex, size_t out_sz);
void encodePhoneNumberPDU(const char* phone, char *out_sw, size_t out_sz, uint8_t *out_digits);
void buildPDU(const char* phone, const char* text, char *out_pdu, size_t out_sz);
void sim_init(UART_HandleTypeDef *uart_sim/*, UART_HandleTypeDef *uart_dbg*/);
void send_SMS(UART_HandleTypeDef *uart_sim,/* UART_HandleTypeDef *uart_dbg, */const char *phone, const char *text);
