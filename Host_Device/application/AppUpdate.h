#ifndef __APPUPDATE_H
#define __APPUPDATE_H

#include "usart.h"

#define APP_VERSION_NUM_ADDR 0X8010000
#define APP_SIZE_ADDR (APP_VERSION_NUM_ADDR+8)
#define APP_START_ADDR (APP_SIZE_ADDR+8)
#define PAGE_SIZE 0X400

uint16_t Receive_app_from_UART(uint8_t *data,uint16_t len);
void Flash_Erase(uint32_t start_addr,uint32_t file_size);

#endif
