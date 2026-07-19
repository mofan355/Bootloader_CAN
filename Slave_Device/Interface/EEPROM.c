#include "EEPROM.h"
#include "i2c.h"
#include "stdio.h"
#include "string.h"

void EEPROM_Write_Byte(uint8_t addr1,uint8_t addr2,uint8_t byte)
{
    uint8_t buf[]={addr1,addr2,byte};
    if(HAL_I2C_Master_Transmit(&hi2c1,DEV_ADDR_W,buf,3,1000)==HAL_OK)
    {
        HAL_Delay(5);
    }
    else printf("EEPROM write ERRO.\r\n");
}

uint8_t EEPROM_Read_Byte(uint16_t addr)
{
    uint8_t byte=0;
    HAL_I2C_Mem_Read(&hi2c1,DEV_ADDR,addr,I2C_MEMADD_SIZE_16BIT,&byte,1,1000);
    return byte;
}

void EEPROM_Write_Bytes(uint16_t addr,uint8_t *data)
{
    for(int i=0;i<strlen((const char*)data);i++)
    {
        uint8_t addr1=addr>>8;
        uint8_t addr2=addr&0x0F;
        EEPROM_Write_Byte(addr1,addr2,data[i]);
        addr++;
    }
}

void EEPROM_Read_Bytes(uint16_t addr,uint8_t buf[],uint16_t read_size)
{
    for(int i=0;i<read_size;i++)
    {
        buf[i]=EEPROM_Read_Byte(addr);
        addr++;
    }
}

void EEPROM_Erase(uint16_t addr,uint16_t erase_size)
{
    for(int i=0;i<erase_size;i++)
    {
        uint8_t addr1=addr>>8;
        uint8_t addr2=addr&0x0F;
        EEPROM_Write_Byte(addr1,addr2,0xFF);
        addr++;
    }
}
