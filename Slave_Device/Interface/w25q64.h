#ifndef __W25Q64_H__
#define __W25Q64_H__

#include "stm32f1xx_hal.h"

#define W25Q64_CS_PORT GPIOB
#define W25Q64_CS_PIN GPIO_PIN_0
#define WRITE_ENABLE 0x06
#define WRITE_DISABLE 0x04
#define READ_STATUS_REGISTER_1 0x05
#define READ_STATUS_REGISTER_2 0x35
#define PAGE_PROGRAM 0x02
#define BLOCK_ERASE_64KB 0xD8
#define BLOCK_ERASE_32KB 0x52
#define SECTOR_ERASE_4KB 0x20
#define CHIP_ERASE 0xC7//0x60
#define JEDEC_ID 0x9F
#define READ_DATA 0x03
#define W25Q64_CS_UP HAL_GPIO_WritePin(W25Q64_CS_PORT,W25Q64_CS_PIN,GPIO_PIN_SET)
#define W25Q64_CS_DOWN HAL_GPIO_WritePin(W25Q64_CS_PORT,W25Q64_CS_PIN,GPIO_PIN_RESET)

HAL_StatusTypeDef W25Q64_SendByte(uint8_t byte);
HAL_StatusTypeDef W25Q64_SendBytes(uint8_t *bytes,uint16_t size);
HAL_StatusTypeDef W25Q64_SendAddr(uint32_t addr);
HAL_StatusTypeDef W25Q64_WaitBUSY(void);
HAL_StatusTypeDef W25Q64_SendCMD(uint8_t cmd);
HAL_StatusTypeDef W25Q64_Erase(uint8_t cmd,uint32_t addr);
void W25Q64_Write(uint32_t addr,uint8_t *data,uint16_t write_size);
void W25Q64_Read(uint32_t addr,uint8_t buf[],uint16_t read_size);

#endif
