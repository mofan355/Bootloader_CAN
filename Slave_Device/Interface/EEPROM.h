#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "main.h"

#define DEV_ADDR 0xA0
#define DEV_ADDR_W 0XA0
#define DEV_ADDR_R 0XA1
#define CURRENT_VERSION_NUM_STRADDR 0X00
#define CURRENT_VERSION_SIZE_STRADDR 0x08
#define BACKUP_VERSION_NUM_STRADDR 0x10
#define BACKUP_VERSION_SIZE_STRADDR 0x18
#define VERSION_NUM_SIZE 0x04
#define APP_SIZE_STRLEN 0x04

void EEPROM_Write_Byte(uint8_t addr1,uint8_t addr2,uint8_t byte);
uint8_t EEPROM_Read_Byte(uint16_t addr);
void EEPROM_Write_Bytes(uint16_t addr,uint8_t *data);
void EEPROM_Read_Bytes(uint16_t addr,uint8_t buf[],uint16_t read_size);
void EEPROM_Erase(uint16_t addr,uint16_t erase_size);

#endif
