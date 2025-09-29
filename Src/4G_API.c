#include "4G_API.h"

/*
 *
 */
/***************************************************************************
 * function name:				- dbg_puts
 *
 * @brief						- This function sends debug message through UART
 *
 * param[in]					- Address of the uart struct address
 * param[in]					- Debug text
 *
 * @return						- None
 * @note						- None
 */

static inline void dbg_puts(UART_HandleTypeDef *uart_dbg, const char *s){
  HAL_UART_Transmit(uart_dbg, (uint8_t*)s, (uint16_t)strlen(s), 100);
}


/*
 *
 */
/***************************************************************************
 * function name:				- sim_writeln
 *
 * @brief						- This function sends AT command through UART that connected to module 4G
 *
 * param[in]					- Address of the uart struct address
 * param[in]					- AT command text
 *
 * @return						- None
 * @note						- None
 */


static inline void sim_writeln(UART_HandleTypeDef *uart_sim, const char *s){
  HAL_UART_Transmit(uart_sim, (uint8_t*)s, (uint16_t)strlen(s), 1000);
  const uint8_t crlf[2] = {'\r','\n'};
  HAL_UART_Transmit(uart_sim, (uint8_t*)crlf, 2, 1000);
}


/*
 *
 */
/***************************************************************************
 * function name:				- sim_wait_result
 *
 * @brief						- This function wait the response of SIM through UART that connected to module 4G
 *
 * param[in]					- Address of the uart struct address
 * param[in]					- timeout
 *
 * @return						- timeout time
 * @note						- None
 */


sim_res_t sim_wait_result(UART_HandleTypeDef *uart_sim, uint32_t total_ms)
{
  uint32_t t0 = HAL_GetTick();
  char win[160] = {0}; size_t wr=0;
  uint8_t b;

  while (HAL_GetTick() - t0 < total_ms){
    if (HAL_UART_Receive(uart_sim, &b, 1, 80) == HAL_OK){
      if (wr+1 < sizeof(win)) { win[wr++] = (char)b; win[wr] = 0; }
      else { memmove(win, win+1, sizeof(win)-2); win[sizeof(win)-2]=(char)b; win[sizeof(win)-1]=0; }

      if (strstr(win, "+CMGS:") || strstr(win, "\nOK")) return SIM_RES_OK;
      if (strstr(win, "+CME ERROR")) return SIM_RES_CME;
      if (strstr(win, "+CMS ERROR")) return SIM_RES_CMS;
      if (strstr(win, "ERROR"))      return SIM_RES_ERROR;
    }
  }
  return SIM_RES_TIMEOUT;
}

/*
 *
 */
/***************************************************************************
 * function name:				- sim_wait_prompt
 *
 * @brief						- This function waits for the ready promt ">>" of the module 4G
 *
 * param[in]					- Address of the uart SIM struct address
 * param[in]					- timeout max
 *
 * @return						- true / false
 * @note						- None
 */

bool sim_wait_prompt(UART_HandleTypeDef *uart_sim, uint32_t max_ms){
  uint32_t t0 = HAL_GetTick();
  char win[96] = {0}; size_t wr = 0;
  uint8_t ch;

  while (HAL_GetTick() - t0 < max_ms){
    if (HAL_UART_Receive(uart_sim, &ch, 1, 80) == HAL_OK){
      if (ch == '>') return true;


      if (wr+1 < sizeof(win)) { win[wr++] = (char)ch; win[wr] = 0; }
      else { memmove(win, win+1, sizeof(win)-2); win[sizeof(win)-2]=(char)ch; win[sizeof(win)-1]=0; }

      if (strstr(win, "+CMS ERROR") || strstr(win, "+CME ERROR") || strstr(win, "\r\nERROR\r\n")){

        return false;
      }
    }
  }
  return false;
}
/*
 *
 */
/***************************************************************************
 * function name:				- sim_cmd_expect_ok
 *
 * @brief						- This function waits for the ready promt "<" of the module 4G
 *
 * param[in]					- Address of the uart SIM struct address
 * param[in]					- expected response
 * param[in]					- timeout max
 *
 * @return						- None
 * @note						- None
 */

void sim_cmd_expect_ok(UART_HandleTypeDef *uart_sim, UART_HandleTypeDef *uart_dbg, const char *cmd, uint32_t wait_ms){
  sim_writeln(uart_sim, cmd);
  sim_res_t r = sim_wait_result(uart_sim, wait_ms);
  if (r == SIM_RES_OK)
	  dbg_puts(uart_dbg, "OK\r\n");
  else if (r == SIM_RES_CME)
	  dbg_puts(uart_dbg, "CME ERROR\r\n");
  else if (r == SIM_RES_CMS)
	  dbg_puts(uart_dbg, "CMS ERROR\r\n");
  else if (r == SIM_RES_ERROR)
	  dbg_puts(uart_dbg, "ERROR\r\n");
  else
	  dbg_puts(uart_dbg, "TIMEOUT\r\n");
}

/*
 *
 */
/***************************************************************************
 * function name:				- utf8ToUcs2Hex
 *
 * @brief						- This function converts the utf8 to ucs2 in hex format
 *
 * param[in]					- string of utf8
 * param[in]					- string in hex format
 * param[in]					- size of hex string
 *
 * @return						- None
 * @note						- None
 */

void utf8ToUcs2Hex(const char* u8, char *out_hex, size_t out_sz){
  size_t wr = 0;
  out_hex[0] ='\0';
  while (*u8 && wr+4 < out_sz){
    uint16_t cp=0x003F;
    uint8_t c = (uint8_t)*u8++;
    if (c < 0x80) cp=c;
    else if((c & 0xE0)==0xC0 && (u8[0] & 0xC0) == 0x80){
    	cp = ((c&0x1F)<<6)|(u8[0]&0x3F);
    	u8++;
    }
    else if((c & 0xF0) == 0xE0 && (u8[0] & 0xC0)==0x80 && (u8[1] & 0xC0) == 0x80){
      cp = ((c & 0x0F) << 12)|((u8[0] & 0x3F) <<6)|(u8[1] & 0x3F); u8 += 2;
    }
    wr += (size_t)snprintf(out_hex+wr, out_sz-wr, "%04X", (unsigned)cp);
  }
}

/*
 *
 */
/***************************************************************************
 * function name:				- encodePhoneNumberPDU
 *
 * @brief						- This function encodes the phone number into PDU format
 *
 * param[in]					- string of utf8
 * param[in]					- string in hex format
 * param[in]					- size of hex string
 *
 * @return						- None
 * @note						- None
 */

void encodePhoneNumberPDU(const char* phone, char *out_sw, size_t out_sz, uint8_t *out_digits){
  char digits[32]; size_t dn=0;
  for (const char*p=phone; *p && dn<sizeof(digits)-1; ++p)

	  if (*p>='0'&&*p<='9') digits[dn++]=*p;

  digits[dn]='\0';

  if (out_digits)
	  *out_digits=(uint8_t)dn;

  if (dn%2)
  {
	  digits[dn++]='F'; digits[dn]=0;
  }
  size_t wr=0;
  for(size_t i=0;i+1<dn && wr+2<out_sz;i+=2)
  {
	  out_sw[wr++]=digits[i+1];
	  out_sw[wr++]=digits[i];
  }
  out_sw[wr]=0;
}
/*
 *
 */
/***************************************************************************
 * function name:				- buildPDU
 *
 * @brief						- This function builds the TPDU format
 *
 * param[in]					- string of phone number
 * param[in]					- string of text that user want to send
 * param[in]					- size of text that user want to send
 *
 * @return						- None
 * @note						- None
 */

void buildPDU(const char* phone, const char* text, char *out_pdu, size_t out_sz){

  char ucs2[4*256];

  utf8ToUcs2Hex(text, ucs2, sizeof(ucs2));

  char numPDU[40];

  uint8_t numlen=0;

  encodePhoneNumberPDU(phone, numPDU, sizeof(numPDU), &numlen);

  char head[32];

  snprintf(head, sizeof head, "00""11""00""%02X", (unsigned)numlen);

  size_t ud_bytes = strlen(ucs2)/2;

  if (ud_bytes>140)
	  ud_bytes=140; // 1 SMS UCS2
  char udl[3];

  snprintf(udl, sizeof udl, "%02X", (unsigned)ud_bytes);
  snprintf(out_pdu, out_sz, "%s""91""%s""00""08""AA""%s""%s", head, numPDU, udl, ucs2);
}
/*
 *
 */
/***************************************************************************
 * function name:				- sim_init
 *
 * @brief						- This function initialize the module 4G by sending the AT command
 *
 * param[in]					- Address of the uart struct address
 * param[in]					- Address of the uart struct address
 *
 * @return						- None
 * @note						- None
 */


void sim_init(UART_HandleTypeDef *uart_sim, UART_HandleTypeDef *uart_dbg){
  HAL_Delay(2500);
  sim_cmd_expect_ok(uart_sim, uart_dbg, "AT",        1500);
  sim_cmd_expect_ok(uart_sim, uart_dbg, "ATE0",      1500);
  sim_cmd_expect_ok(uart_sim, uart_dbg, "AT+CMGF=0", 1500);
}

/*
 *
 */
/***************************************************************************
 * function name:				- send_SMS
 *
 * @brief						- This function sends pdu code through uart into module SIM
 *
 * param[in]					- Address of the uart struct address
 * param[in]					- Address of the uart struct address
 * param[in]					- Text of phone number
 * param[in]					- Text that user want to send
 *
 * @return						- None
 * @note						- None
 */

void send_SMS(UART_HandleTypeDef *uart_sim, UART_HandleTypeDef *uart_dbg, const char *phone, const char *text)
{
  char pdu[640];
  buildPDU(phone, text, pdu, sizeof(pdu));
  size_t total_bytes = strlen(pdu)/2; if (!total_bytes)
	  return;
  int tpdu_len = (int)(total_bytes - 1); if (tpdu_len <= 0)
	  return;

  char cmd[32]; snprintf(cmd, sizeof cmd, "AT+CMGS=%d", tpdu_len);
  sim_writeln(uart_sim, cmd);

  if (!sim_wait_prompt(uart_sim, 5000)){
    dbg_puts(uart_dbg, "NO_PROMPT\r\n");

    (void)sim_wait_result(uart_sim, 3000);

    sim_cmd_expect_ok(uart_sim, uart_dbg, "AT", 1000);
    HAL_Delay(300);
    return;
  }

  HAL_UART_Transmit(uart_sim, (uint8_t*)pdu, (uint16_t)strlen(pdu), 3000);
  uint8_t SUB=0x1A;
  HAL_UART_Transmit(uart_sim, &SUB, 1, 1000);


  sim_res_t r = sim_wait_result(uart_sim, 15000);
  if (r == SIM_RES_OK)          dbg_puts(uart_dbg, "OK\r\n");
  else if (r == SIM_RES_CMS)    dbg_puts(uart_dbg, "CMS ERROR\r\n");
  else if (r == SIM_RES_CME)    dbg_puts(uart_dbg, "CME ERROR\r\n");
  else if (r == SIM_RES_ERROR)  dbg_puts(uart_dbg, "ERROR\r\n");
  else                          dbg_puts(uart_dbg, "TIMEOUT\r\n");


  uint8_t d; uint32_t t0 = HAL_GetTick();
  while (HAL_GetTick() - t0 < 200) HAL_UART_Receive(uart_sim, &d, 1, 10);
  HAL_Delay(300);
}
