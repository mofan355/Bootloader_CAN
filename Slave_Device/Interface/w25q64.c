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
    W25Q64_SendBytes(data,write_size);
    W25Q64_CS_UP;
}

void W25Q64_Read(uint32_t addr,uint8_t *buf,uint32_t read_size)
{
    W25Q64_CS_DOWN;
    W25Q64_SendByte(READ_DATA);
    W25Q64_SendAddr(addr);
    HAL_SPI_Receive(&hspi1,buf,read_size,1000);
    W25Q64_CS_UP;
}

void W25Q64_AutoErase(uint32_t addr,uint32_t size)
{
    uint32_t first_sector_addr=addr&0xFFF000;
    uint32_t last_sector_addr=(addr+size-1)&0xFFF000;

    //根据一个块内需擦除的sector个数选择擦除模式
    uint16_t sectors=0;//记录当前块内sector个数
    for(uint32_t i=first_sector_addr;i<=last_sector_addr;i+=W25Q64_SECTOR_SIZE)
    {
        sectors++;
        //遍历到当前块的最后一个sector
        if(i&0xF000==0xF000||i==last_sector_addr)
        {
            //如果个数大于或等于一个块内的sector总数量的一半
            if(sectors>=8)
            {
                //如果个数等于一个块内的sector总数量，使用64KB擦除模式
                if(sectors==16) 
                {
                    W25Q64_Erase(BLOCK_ERASE_64KB,i);
                    W25Q64_WaitBUSY();
                }
                //如果个数等于一个块内的sector总数量的一半，使用32KB擦除模式
                else if(sectors==8) 
                {
                    W25Q64_Erase(BLOCK_ERASE_32KB,i);
                    W25Q64_WaitBUSY();
                }
                //如果个数大于一个块内的sector总数量的一半
                else
                {
                    if(i&0xF000==0xF000)
                    {
                        W25Q64_Erase(BLOCK_ERASE_32KB,i);
                        W25Q64_WaitBUSY();

                        for(int j=first_sector_addr;j&0xF000<=0x7000;j+=W25Q64_SECTOR_SIZE)
                        {
                            W25Q64_Erase(SECTOR_ERASE_4KB,j);
                            W25Q64_WaitBUSY();
                        }
                    }
                    else if(i==last_sector_addr)
                    {
                        for(uint8_t j=0;j<sectors-8;j++)
                        {
                            W25Q64_Erase(SECTOR_ERASE_4KB,i-j*W25Q64_SECTOR_SIZE);
                            W25Q64_WaitBUSY();
                        }
                        W25Q64_Erase(BLOCK_ERASE_32KB,i-8*W25Q64_SECTOR_SIZE);
                        W25Q64_WaitBUSY();
                    }
                }
            }
            //如果小于，则直接使用4KB擦除模式
            else
            {
                for(uint8_t j=0;j<sectors;j++)
                {
                    W25Q64_Erase(SECTOR_ERASE_4KB,i-j*W25Q64_SECTOR_SIZE);
                    W25Q64_WaitBUSY();
                }
            }
            sectors=0;
        }
    }
}

void W25Q64_PagesWrite(uint32_t addr,uint8_t *data,uint32_t write_size)
{
    uint8_t addr_in_first_page=addr&0xFF;
    uint8_t addr_in_last_page=(addr+write_size-1)&0xFF;
    //不跨页，直接编写
    if((addr&0xFFFF00)==((addr+write_size-1)&0xFFFF00))
    {
        W25Q64_Write(addr,data,write_size);
        W25Q64_WaitBUSY();
    }
    //跨页写
    else
    {
        uint8_t *temp;

        //首页数据写入
        W25Q64_Write(addr,data,0x100-addr_in_first_page);
        W25Q64_WaitBUSY();
        //地址偏移
        addr+=0x100-addr_in_first_page;
        temp=&data[0x100-addr_in_first_page];

        //计算中间整页部分需编写次数
        uint16_t program_times=0;
        uint32_t mid_size=write_size-(0x100-addr_in_first_page)-(addr_in_last_page+1);
        program_times=mid_size/W25Q64_PAGE_SIZE;

        // printf("\r\naddr_in_first_page %x\r\n",addr_in_first_page);
        // printf("addr_in_last_page %x\r\n",addr_in_last_page);
        // printf("addr %x\r\n",addr);
        // printf("program_times %d\r\n",program_times);
        
        for(int i=0;i<program_times;i++)
        {
            //写入
            W25Q64_Write(addr,temp,W25Q64_PAGE_SIZE);
            W25Q64_WaitBUSY();
            addr+=W25Q64_PAGE_SIZE;
            temp=&temp[W25Q64_PAGE_SIZE];
        }

        //尾页数据写入
        W25Q64_Write(addr,temp,addr_in_last_page+1);
        W25Q64_WaitBUSY();
    }

}

void W25Q64_Test(void)
{
    uint32_t spi_addr=0x01;
    uint8_t spi_data[512]={0xA5,0x5A};
		for(int i=0;i<512;i++)
		{
			if(i%2==0) spi_data[i]=0xA5;
			else spi_data[i]=0x5A;
		}
    uint8_t spi_buf[518]={0};
    uint32_t write_size=512;

    W25Q64_AutoErase(spi_addr,write_size+5);
    W25Q64_Read(spi_addr,spi_buf,write_size+5);
    printf("\r\nafter erasing\r\n");
    for(int i=0;i<write_size;i++)
    {
        printf("%x ",spi_buf[i]);
				if(i>0&&((i+1)%50)==0) printf("\r\n");
    }
    W25Q64_PagesWrite(spi_addr,spi_data,write_size);


//   W25Q64_Erase(SECTOR_ERASE_4KB,spi_addr);
//   W25Q64_WaitBUSY();
//   printf("sector erase finished.\r\n");
//   W25Q64_Write(spi_addr,spi_data,2);
//   W25Q64_WaitBUSY();
    W25Q64_Read(spi_addr,spi_buf,write_size+5);
    printf("after writing\r\n");
    for(int i=0;i<write_size+5;i++)
    {
        printf("%x ",spi_buf[i]);
				if(i>0&&((i+1)%50)==0) printf("\r\n");
    }
}
