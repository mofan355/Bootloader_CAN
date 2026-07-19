#include "w25q64.h"
#include "spi.h"
#include "stdio.h"

void W25Q64_SendCMD(uint8_t data)
{
    if(HAL_SPI_Transmit(&hspi1,&data,1,1000)!=HAL_OK)
    {
        printf("W25Q64_SendCMD ERRO");
    }
}

void W25Q64_WriteInit(void)
{
    W25Q64_CS_DOWN;
    W25Q64_SendCMD(WRITE_ENABLE);
    W25Q64_SendCMD(PAGE_PROGRAM);
}

void W25Q64_Read(uint32_t addr,uint8_t buf[],uint16_t read_size)
{
    W25Q64_SendCMD(READ_DATA);
    uint8_t addr_buf[]={addr>>16,(addr>>8)&0xFF,addr&0xFF};
    HAL_SPI_Transmit(&hspi1,addr_buf,4,1000);
    HAL_SPI_Receive(&hspi1,buf,read_size,1000);
}
