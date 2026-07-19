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
  hcan.Init.Prescaler = 36;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_6TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = ENABLE;
  hcan.Init.AutoRetransmission = DISABLE;
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

void CAN_ReceiveMsg(RxMsg rxMsg[],uint16_t *MsgCount)
{
  //等待接收缓冲区有数据
  while(HAL_CAN_GetRxFifoFillLevel(&hcan,CAN_RX_FIFO0)==0);
	printf("start receive\r\n");
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

void CNA_Receive_App_to_FLASH(uint16_t app_size)
{
  uint16_t receive_count=0;

  for(int i=0;i<receive_count;i++)
  {

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
