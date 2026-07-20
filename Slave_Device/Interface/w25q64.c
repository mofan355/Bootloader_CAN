#include "w25q64.h"
#include "spi.h"
#include "stdio.h"

HAL_StatusTypeDef W25Q64_SendByte(uint8_t byte)
{
    return HAL_SPI_Transmit(&hspi1,&byte,1,1000);
}

HAL_StatusTypeDef W25Q64_SendBytes(uint8_t *bytes,uint16_t size)
{
    return HAL_SPI_Transmit(&hspi1,bytes,size,1000);
}

HAL_StatusTypeDef W25Q64_SendAddr(uint32_t addr)
{
    uint8_t addr_buf[3]={(addr>>16)&0xFF,(addr>>8)&0xFF,addr&0xFF};
    return W25Q64_SendBytes(addr_buf,3);
}

HAL_StatusTypeDef W25Q64_WaitBUSY(void)
{
    W25Q64_CS_DOWN;
    uint8_t buf=1;
    W25Q64_SendByte(READ_STATUS_REGISTER_1);
    while(buf&0x01)
    {
        HAL_SPI_Receive(&hspi1,&buf,1,1000);
//        printf("%x\r\n",buf);
    }
    W25Q64_CS_UP;
    return HAL_OK;
}

HAL_StatusTypeDef W25Q64_SendCMD(uint8_t cmd)
{
    W25Q64_CS_DOWN;
    //发送cmd
    HAL_StatusTypeDef result=HAL_SPI_Transmit(&hspi1,&cmd,1,1000);
    if(result!=HAL_OK)
    {
        printf("W25Q64 send cmd ERRO %d\r\n",result);
    }
    W25Q64_CS_UP;
    return result;
}

HAL_StatusTypeDef W25Q64_Erase(uint8_t cmd,uint32_t addr)
{
    W25Q64_SendCMD(WRITE_ENABLE);

    W25Q64_CS_DOWN;
    W25Q64_SendByte(cmd);
    W25Q64_SendAddr(addr);
    W25Q64_CS_UP;
	return HAL_OK;
}

void W25Q64_Write(uint32_t addr,uint8_t *data,uint16_t write_size)
{
    W25Q64_SendCMD(WRITE_ENABLE);
    
    W25Q64_CS_DOWN;
    W25Q64_SendByte(PAGE_PROGRAM);
    W25Q64_SendAddr(addr);
    //跨页写功能待补全
    W25Q64_SendBytes(data,write_size);
    W25Q64_CS_UP;
}

void W25Q64_Read(uint32_t addr,uint8_t *buf,uint16_t read_size)
{
    W25Q64_CS_DOWN;
    W25Q64_SendByte(READ_DATA);
    W25Q64_SendAddr(addr);
    HAL_SPI_Receive(&hspi1,buf,read_size,1000);
    W25Q64_CS_UP;
}
