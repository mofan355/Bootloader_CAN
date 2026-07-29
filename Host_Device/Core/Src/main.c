/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdlib.h"
#include "AppUpdate.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define stdId 0x123
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  CAN_FilterConfig();
  HAL_CAN_Start(&hcan);
	
	printf("current version in host:");
  for(int i=0;i<4;i++)
  {
    printf("%c",*((volatile uint8_t *)(APP_VERSION_NUM_ADDR+i)));
  }
  printf("\r\nsize:%lu\r\n",*((volatile uint32_t *)APP_SIZE_ADDR));
	
  printf("host start...\r\n");
  uint8_t *host_version=NULL;
  uint8_t *slave_version=NULL;
	//向从机请求从机当前版本信息
  uint8_t *data="version";
  CAN_SendMsg_long(stdId,data,strlen((const char*)data));
  printf("access information of current version of slave.\r\n");

  RxMsg msg[8]={0};
  uint16_t MsgCount=0;
  CAN_ReceiveMsg(msg,&MsgCount);
  slave_version=msg[0].data;
  printf_Infor_from_CAN(msg,MsgCount);
	
  //向电脑端获取最新版本信息
  printf("access latest version\r\n");
  uint8_t rxMsg[512]={0};
  uint16_t rxLen=0;
  rxLen=Receive_Info_from_UART(rxMsg,4);
  printf("uart receive data: %s-->%d\r\n",rxMsg,rxLen);
  //暂存最新版本号
  uint8_t last_version[4]={0};
  memcpy(last_version,rxMsg,4);

  //存储从机版本与最新版本的比较结果
  uint8_t *compare_result="NO";
  //比较slave版本和最新版本是否一致
	printf("start compare version between slave and latest:Need update(YES)/No need(NO)\r\n");
  // if(strncmp((const char*)slave_version,(const char*)rxMsg,4)==0)
  //一致，不需要更新
  if(memcmp(slave_version,rxMsg,4)==HAL_OK)
  {
		printf("Slave no need update.\r\n");
		//发送版本比较结果给从机，YES表示需要更新，NO表示不需要更新
		CAN_SendMsg_long(stdId,compare_result,strlen((const char*)compare_result));
		printf("send '%s' successfully.\r\n",compare_result);
  }
  //不一致，需要更新
  else
  {
		//更新主机中的版本
		
    compare_result="YES";
    printf("Slave need update.\r\n");
    printf("Starting update host and compare version between host and latest.\r\n");
		
		//比较主机中的版本与最新版本是否一致
    //从flash中读取host中存有的app的版本号
    host_version=(volatile uint8_t *)APP_VERSION_NUM_ADDR;
    // if(strncmp((const char*)host_version,(const char*)rxMsg,4)==0)
    //一致，不需要更新host现存版本
    if(memcmp(host_version,rxMsg,4)==HAL_OK)
    {
      printf("No need to update app of host\r\n");
    }
    //不一致，需要更新host现存版本
    else
    {
      printf("Start to update app of host\r\n");
      //得到新程序大小Byte
      printf("Inputing file size(Byte) please:\r\n");
      Receive_Info_from_UART(rxMsg,16);
      rxLen=atoi((const char*)rxMsg);
      printf("file size:%d Byte\r\n",rxLen);
      //擦除flash
      printf("start erase special flash\r\n");
      Flash_Erase(APP_VERSION_NUM_ADDR,rxLen);
      printf("flash erase successfully.\r\n");

      //存储最新app的版本和大小
      while(HAL_FLASH_Unlock()!=HAL_OK);
      //存储app的版本号
      //暂存新的版本号
      uint32_t vn=((uint32_t)last_version[3]<<24)|((uint32_t)last_version[2]<<16)|((uint32_t)last_version[1]<<8)|last_version[0];
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,APP_VERSION_NUM_ADDR,vn);
	    //存储app大小
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,APP_SIZE_ADDR,rxLen);
      HAL_FLASH_Lock();

      //通过uart从电脑接收新版本程序并写入到flash
			printf("start receive app from UART to host flash\r\n");
      rxLen=Receive_app_from_UART(rxMsg,rxLen);
      printf("received app:%d Byte.\r\n",rxLen);
      printf("Host have updated.\r\n");
    }
		
		//发送版本比较结果给从机，YES表示需要更新，NO表示不需要更新
		CAN_SendMsg_long(stdId,compare_result,strlen((const char*)compare_result));
		printf("send '%s' successfully.\r\n",compare_result);

		//读取host的flash中的最新版本程序并发送给slave
		//等待从机备份好，发来start1再开始发送
		printf("waitting 'start1' of slave.\r\n");
		while(1)
		{
			CAN_ReceiveMsg(msg,&rxLen);
			printf_Infor_from_CAN(msg,rxLen);
			if(strncmp((const char *)msg[0].data,"start1",6)==0) break;
			else printf("Retry please.");
		}

		//开始发送
		printf("start send app from host flash to slave\r\n");
    //发送数据版本号
    printf("current version:");
    for(int i=0;i<4;i++)
    {
      printf("%c",last_version[i]);
    }
    printf("\r\nSending app version num.\r\n");
    CAN_SendMsg_long(stdId,last_version,4);
    printf("Sending app version num successfully\r\n");

		//发送数据大小Byte
		uint32_t app_size=*((volatile uint32_t *)APP_SIZE_ADDR);
		printf("current app_size:%lu Byte\r\n",app_size);
		printf("Sending app size.\r\n");
		uint8_t temp_arr[2]={(uint8_t)(app_size&0xFF),(uint8_t)(app_size>>8)};
		CAN_SendMsg_long(stdId,temp_arr,2);
		printf("Sending app size successfully\r\n");

    //等待从机备份好，发来start2再开始发送
		printf("waitting 'start2' of slave.\r\n");
		while(1)
		{
			CAN_ReceiveMsg(msg,&rxLen);
			printf_Infor_from_CAN(msg,rxLen);
			if(strncmp((const char *)msg[0].data,"start2",6)==0) break;
			else printf("Retry please.");
		}

		//发送数据内容
		printf("Starting send app content...\r\n");
		CAN_Send_App_From_Flash(stdId,app_size);
		printf("send app finished!\r\n");
  }
  printf("bootloader finished.\r\n");
  printf("last version:");
  for(int i=0;i<4;i++)
  {
    printf("%c",*((volatile uint8_t *)(APP_VERSION_NUM_ADDR+i)));
  }
  printf("\r\nsize:%lu",*((volatile uint32_t *)APP_SIZE_ADDR));
  
  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
