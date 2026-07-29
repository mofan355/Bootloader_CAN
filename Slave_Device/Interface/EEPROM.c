#include "EEPROM.h"
#include "i2c.h"
#include "stdio.h"
#include "string.h"

void EEPROM_Write_Byte(uint8_t addr1,uint8_t addr2,uint8_t byte)
{
    uint8_t buf[]={addr1,addr2,byte};
    if(HAL_I2C_Master_Transmit(&hi2c1,DEV_ADDR_W,buf,3,1000)==HAL_OK)
    {
       uint8_t ack=3;
       while(ack!=0)
       {
           HAL_I2C_Master_Receive(&hi2c1,DEV_ADDR_R,&ack,1,1000);
       }
			// HAL_Delay(5);
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
    for(int i=0;i<sizeof(data);i++)
    {
        uint8_t addr1=addr>>8;
        uint8_t addr2=addr&0xFF;
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
        uint8_t addr2=addr&0xFF;
        EEPROM_Write_Byte(addr1,addr2,0xFF);
        addr++;
    }
}

void EEPROM_Test(void)
{
    uint8_t buf[36]={0};
    EEPROM_Erase(CURRENT_VERSION_NUM_STRADDR,32);

    EEPROM_Read_Bytes(CURRENT_VERSION_NUM_STRADDR,buf,36);
    printf("after erase\r\n");
    for(int i=0;i<36;i++)
    {
        printf("%x ",buf[i]);
        if((i+1)%8==0) printf("\r\n");
    }

    uint8_t *data="v1.1";
    EEPROM_Write_Bytes(CURRENT_VERSION_NUM_STRADDR,data);
    data="v1.2";
    EEPROM_Write_Bytes(BACKUP_VERSION_NUM_STRADDR,data);
		
    uint32_t size=0x4FCD;
    //(size>>24)&0xFF,(size>>16)&0xFF,(size>>8)&0xFF,size&0xFF
    //size&0xFF,(size>>8)&0xFF,(size>>16)&0xFF,(size>>24)&0xFF
    uint8_t size_buf[4]={(size>>24)&0xFF,(size>>16)&0xFF,(size>>8)&0xFF,size&0xFF};
    EEPROM_Write_Bytes(CURRENT_VERSION_SIZE_STRADDR,size_buf);
    size=0x5ABCD;
		uint8_t size_buf2[4]={(size>>24)&0xFF,(size>>16)&0xFF,(size>>8)&0xFF,size&0xFF};
    EEPROM_Write_Bytes(BACKUP_VERSION_SIZE_STRADDR,size_buf2);

    EEPROM_Read_Bytes(CURRENT_VERSION_NUM_STRADDR,buf,36);
    printf("after write\r\n");
    printf("%c%c%c%c\r\n",buf[0],buf[1],buf[2],buf[3]);
    printf("%c%c%c%c\r\n",buf[16],buf[17],buf[18],buf[19]);
    for(int i=0;i<36;i++)
    {
        printf("%x ",buf[i]);
        if((i+1)%8==0) printf("\r\n");
    }
}
