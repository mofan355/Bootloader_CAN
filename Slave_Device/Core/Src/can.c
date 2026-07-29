/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */
#include "stdio.h"
#include "string.h"
#include "app.h"
/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 24;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = ENABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void CAN_FilterConfig(void)
{
  CAN_FilterTypeDef filterConfig;
  filterConfig.FilterFIFOAssignment=CAN_FILTER_FIFO0;
  filterConfig.FilterBank=0;
  filterConfig.FilterMode=CAN_FILTERMODE_IDMASK;
  filterConfig.FilterScale=CAN_FILTERMODE_IDMASK;
  filterConfig.FilterIdHigh=0x00;
  filterConfig.FilterIdLow=0x00;
  filterConfig.FilterMaskIdHigh=0x00;
  filterConfig.FilterMaskIdLow=0x00;
  filterConfig.FilterActivation=CAN_FILTER_ENABLE;
  HAL_CAN_ConfigFilter(&hcan,&filterConfig);
}

void CAN_SendMsg(uint32_t stdId,uint8_t *data,uint16_t len)
{
  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan)==0);
  CAN_TxHeaderTypeDef txHeader;
  txHeader.StdId=stdId;
  txHeader.DLC=len;
  txHeader.IDE=CAN_ID_STD;
  txHeader.RTR=CAN_RTR_DATA;
  uint32_t txMailtBox;
  HAL_CAN_AddTxMessage(&hcan,&txHeader,data,&txMailtBox);
  
  if(txMailtBox==CAN_TX_MAILBOX0)
  {
    while(__HAL_CAN_GET_FLAG(&hcan,CAN_FLAG_TXOK0)==0);
  }
  else if(txMailtBox==CAN_TX_MAILBOX1)
  {
    while(__HAL_CAN_GET_FLAG(&hcan,CAN_FLAG_TXOK1)==0);
  }
  else if(txMailtBox==CAN_TX_MAILBOX0)
  {
    while(__HAL_CAN_GET_FLAG(&hcan,CAN_FLAG_TXOK2)==0);
  }
}

void CAN_SendMsg_long(uint32_t stdId,uint8_t *data,uint16_t len)
{
  uint16_t send_count=(uint16_t)(len/8);
  if(len%8!=0) send_count++;
  uint8_t lowByte=(uint8_t)(send_count&0xFF);
  uint8_t highByte=(uint8_t)(send_count>>8);
  uint8_t temp_arry[3]={lowByte,highByte};
  CAN_SendMsg(stdId,temp_arry,2);

  for(uint16_t j=0;j<len;j+=8)
  {
    uint8_t temp[8]={0};
    uint32_t send_len=0;
    if(len-j>=8) send_len=8;
    else send_len=len-j;
    memcpy(temp,data+j,send_len);
    CAN_SendMsg(stdId,temp,send_len);
  }
}

/**
*@brief CAN接收报文
*
*@param rxMsg 用来存储报文的数组，每一位可存储一个报文
*
*@param MsgCount 接收到的报文个数，一个报文最多8 bytes
*/
void CAN_ReceiveMsg(RxMsg rxMsg[],uint16_t *MsgCount)
{
  //等待接收缓冲区有数据
  while(HAL_CAN_GetRxFifoFillLevel(&hcan,CAN_RX_FIFO0)==0);
	// printf("start receive\r\n");
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t temp[8]={0};
  //得到报文数量
  HAL_CAN_GetRxMessage(&hcan,CAN_RX_FIFO0,&rxHeader,temp);
  *MsgCount=temp[0]|(temp[1]<<8);
  // printf("MsgCount-->%dseconds\r\n",*MsgCount);
  //接收报文内容
  for(int i=0;i<*MsgCount;i++)
  {
    //等待接收缓冲区有数据
    while(HAL_CAN_GetRxFifoFillLevel(&hcan,CAN_RX_FIFO0)==0);
    HAL_CAN_GetRxMessage(&hcan,CAN_RX_FIFO0,&rxHeader,rxMsg[i].data);
    rxMsg[i].stdId=rxHeader.StdId;
    rxMsg[i].len=rxHeader.DLC;
  }
}

/**
 * @brief CAN接收app内容并存储到flash中
 * 
 * @param app_size 本次app大小
 */
void CAN_Receive_App_to_FLASH(uint16_t app_size)
{
  //计算需接收次数，每次接收1KB
  uint16_t receive_count=app_size/1024;
  if(app_size%1024!=0) receive_count+=1;
  RxMsg rxMsg[128]={0};
	uint16_t MsgCount=0;
  uint32_t addr=APP_START_ADDR_FLASH;
	uint16_t received_size=0;
  for(uint16_t i=0;i<receive_count;i++)
  {
    //接收报文
    CAN_ReceiveMsg(rxMsg,&MsgCount);

    //写入flash
    for(int j=0;j<MsgCount;j++)
    {
      //根据最后一条报文的数据长度，选择最佳写入模式
      if(j+1==MsgCount)
      {
        //如果长度为8，以FLASH_TYPEPROGRAM_DOUBLEWORD模式写入
        if(rxMsg[j].len==8) 
        {
          Write_Flash(FLASH_TYPEPROGRAM_DOUBLEWORD,addr
            ,rxMsg[j].data,rxMsg[j].len);
        }
        //否则，以FLASH_TYPEPROGRAM_HALFWORD模式写入
        else 
        {
          Write_Flash(FLASH_TYPEPROGRAM_HALFWORD,addr
            ,rxMsg[j].data,rxMsg[j].len);
        }
      }
      //其他报文以FLASH_TYPEPROGRAM_DOUBLEWORD模式快速写入，每次写入rxMsg[j].len bytes
      else 
      {
          Write_Flash(FLASH_TYPEPROGRAM_DOUBLEWORD,addr
            ,rxMsg[j].data,rxMsg[j].len);
            
          //地址偏移,rxMsg[j].len通常为8
          addr+=rxMsg[j].len;
      }
			received_size+=rxMsg[j].len;
    }
		
	printf("%d\r\n",received_size);
  }
}

void printf_Infor_from_CAN(RxMsg rxMsg[],uint16_t MsgCount)
{
  for(int i=0;i<MsgCount;i++)
    {
      if(i==0) printf("slave r:");
      for(int j=0;j<rxMsg[i].len;j++)
      {
        printf("%c",rxMsg[i].data[j]);
      }
      if(i+1==MsgCount) printf("\r\n");
    }
}
/* USER CODE END 1 */
