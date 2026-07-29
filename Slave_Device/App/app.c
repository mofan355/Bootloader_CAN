#include "app.h"
#include "can.h"
#include "usart.h"
#include "i2c.h"
#include "spi.h"

/**
 * @brief 根据要擦除的大小以页擦除的模式擦除flash
 * 
 * @param start_addr 擦除的起始地址
 * @param erase_size 要擦除的大小
 */
void Flash_Erase(uint32_t start_addr,uint32_t erase_size)
{
    //计算需要擦除的页数
    uint8_t page_count=erase_size/FLASH_PAGE_SIZE;
    if(erase_size%FLASH_PAGE_SIZE!=0) page_count++;

    //擦除flash
    HAL_FLASH_Unlock();
    uint8_t clear_flag=0;
    for(int i=0;i<page_count;i++)
    {
        uint32_t page_addr=start_addr+i*FLASH_PAGE_SIZE;
        //判断是否需要擦除
        for(int j=0;j<FLASH_PAGE_SIZE;j++)
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

/**
 * @brief 根据mode，以addr为起始地址，写入数据到flash中
 * 
 * @param mode 写入模式
 *             FLASH_TYPEPROGRAM_HALFWORD
 *             FLASH_TYPEPROGRAM_WORD
 *             FLASH_TYPEPROGRAM_DOUBLEWORD
 * @param addr 起始写入位置的地址
 * @param data 要写入的数据
 * @param size 写入数据的大小
 */
void Write_Flash(uint8_t mode,uint32_t addr,uint8_t *data,uint16_t size)
{
    while(HAL_FLASH_Unlock()!=HAL_OK);
    if(mode==FLASH_TYPEPROGRAM_HALFWORD)
    {
        for(uint32_t i=0;i<size;i+=2)
        {
            HAL_FLASH_Program(mode,addr+i,((uint64_t)data[i+1]<<8)|data[i]);
        }
    }
    else if(mode==FLASH_TYPEPROGRAM_WORD)
    {
        for(uint32_t i=0;i<size;i+=4)
        {
            HAL_FLASH_Program(mode,addr+i,((uint64_t)data[i+3]<<24)|((uint64_t)data[i+2]<<16)|((uint64_t)data[i+1]<<8)|data[i]);
        }
    }
    else if (mode==FLASH_TYPEPROGRAM_DOUBLEWORD)
    {
        for(uint32_t i=0;i<size;i+=8)
        {
            HAL_FLASH_Program(mode,addr+i,
                ((uint64_t)data[i+7]<<56)|((uint64_t)data[i+6]<<48)|((uint64_t)data[i+5]<<40)|((uint64_t)data[i+4]<<32)|
                ((uint64_t)data[i+3]<<24)|((uint64_t)data[i+2]<<16)|((uint64_t)data[i+1]<<8)|data[i]);
        }
    }
    
    HAL_FLASH_Lock();
}

void jump(void)
{
	
  printf("Starting jump to app.\r\n");
    //停止所有外设
    HAL_CAN_Stop(&hcan);
	HAL_CAN_DeInit(&hcan);
	HAL_UART_DeInit(&huart1);
	HAL_I2C_DeInit(&hi2c1);
	HAL_SPI_DeInit(&hspi1);

    //关闭SysTick,清楚时基
    SysTick->CTRL=0;
    SysTick->LOAD=0;
    SysTick->VAL=0;

    //关闭所有外设中断，清楚所有挂起标志
    for(uint8_t i=0;i<8;i++)
    {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }

    //复位时钟系统
    HAL_RCC_DeInit();

	//关闭中断
    __disable_irq();
    //设置向量表
    SCB->VTOR=APP_START_ADDR_FLASH;
    //定义指针函数APP_START_ADDR_FLASH+4(复位中断)
    typedef void (*app_entry)(void);
    //创建指针函数并指向
    app_entry app_reset=(app_entry)(*(volatile uint32_t*)(APP_START_ADDR_FLASH+4));
    //设置主栈指针
    __set_MSP(*((volatile uint32_t*)APP_START_ADDR_FLASH));
    //跳转到APP复位函数
    app_reset();
}
