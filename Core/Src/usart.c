/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

static volatile bool bt_tx_busy = false;
static char bt_tx_buf[64];

/* USART1 init function */
void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX;        // ???!
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uartHandle->Instance==USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        // ??? TX (PA9)
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // ??? PA10,??????????

        HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);  // ???2,??TIM2
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    if(uartHandle->Instance==USART1)
    {
        __HAL_RCC_USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
}

/* USER CODE BEGIN 1 */

void BT_SendData(uint16_t count, uint16_t alarm, uint8_t state)
{
    if(bt_tx_busy) return;
    
    snprintf(bt_tx_buf, sizeof(bt_tx_buf), "COUNT:%04d,ALARM:%04d,STATE:%d\r\n", 
             count, alarm, state);
    
    bt_tx_busy = true;
    HAL_UART_Transmit_IT(&huart1, (uint8_t*)bt_tx_buf, strlen(bt_tx_buf));
}

void BT_SendString(const char *str)
{
    if(bt_tx_busy) return;
    
    strncpy(bt_tx_buf, str, sizeof(bt_tx_buf)-1);
    bt_tx_buf[sizeof(bt_tx_buf)-1] = '\0';
    
    bt_tx_busy = true;
    HAL_UART_Transmit_IT(&huart1, (uint8_t*)bt_tx_buf, strlen(bt_tx_buf));
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        bt_tx_busy = false;
    }
}

/* USER CODE END 1 */