#include "AppUpdate.h"
#include "stm32f1xx_hal.h"

uint16_t Receive_app_from_UART(uint8_t *data,uint16_t len)
{
	//存储app大小到前四个字节
  while(HAL_FLASH_Unlock()!=0);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,APP_START_ADDR,len);
  HAL_FLASH_Lock();

  uint16_t receive_count=len/256;
  if(len%256!=0) receive_count++;
  uint16_t total_len=0;
  for(int i=0;i<receive_count;i++)
  {
    uint16_t temp_len=Receive_Info_from_UART(data,256);
    printf("receive %d-->%d Byte\r\n",i,temp_len);
    total_len+=temp_len;
    if(temp_len!=256)
    {
      if(temp_len!=len%256)
      {
        printf("receive data error\r\n");
        return total_len;
      }
    }
    //写入flash
    while(HAL_FLASH_Unlock()!=0);

    for(int j=0;j<temp_len;j+=2)
    {
      //前四个字节用来存储app大小，app内容从第四个字节开始写
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,APP_RUN_ADDR+i*256+j,(data[j+1]<<8)|data[j]);
    }

    HAL_FLASH_Lock();
  }
  return total_len;
}

void Flash_Erase(uint32_t start_addr,uint16_t file_size)
{
    //计算需要擦除的页数
    uint8_t page_count=file_size/PAGE_SIZE;
    if(file_size%PAGE_SIZE!=0) page_count++;

    //擦除flash
    HAL_FLASH_Unlock();
    uint8_t clear_flag=0;
    for(int i=0;i<page_count;i++)
    {
        uint32_t page_addr=start_addr+i*PAGE_SIZE;
        //判断是否需要擦除
        for(int j=0;j<PAGE_SIZE;j++)
        {
            if(*((volatile uint8_t *)page_addr+j)!=0xFF)
            {
                clear_flag=1;
                break;
            }
        }

        if(clear_flag==1)
        {
          //开始擦除flash
          FLASH_EraseInitTypeDef erase_init;
          erase_init.TypeErase=FLASH_TYPEERASE_PAGES;
          erase_init.PageAddress=(uint32_t)page_addr;
          erase_init.NbPages=1;

					uint32_t pageErro=0;
          HAL_FLASHEx_Erase(&erase_init,&pageErro);
					printf("pageErro-->%d\r\n",pageErro);

          clear_flag=0;
          //等待擦除完成
          HAL_Delay(50);
        }
        else continue;
    }
    HAL_FLASH_Lock();
}
