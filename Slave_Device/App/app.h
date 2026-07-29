#ifndef __APP_H__
#define __APP_H__

#include "stm32f1xx_hal.h"

#define APP_START_ADDR_FLASH 0x8006000
#define APP_MAX_SIZE 0xA000
#define APP_START_ADDR_BACKUP 0x00

void Flash_Erase(uint32_t start_addr,uint32_t erase_size);
void Write_Flash(uint8_t mode,uint32_t addr,uint8_t *data,uint16_t size);
void jump(void);

#endif
