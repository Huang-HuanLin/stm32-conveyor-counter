/* USER CODE BEGIN Header */
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "oled.h"
#include "segment.h"

/* Private function prototypes */
void SystemClock_Config(void);
void Error_Handler(void);

/* ????(???static,?????) */
uint16_t item_count = 0;
uint16_t alarm_count = 0;
uint8_t last_sensor = 0;
uint8_t stable_sensor = 0;
uint8_t debounce_cnt = 0;
uint16_t block_timer = 0;
uint8_t alarm_flag = 0;
uint8_t oled_refresh_flag = 1;
uint8_t prev_stable = 0;
uint8_t last_alarm_flag = 0;

#define BLOCK_THRESHOLD  200
#define DEBOUNCE_THRESHOLD 10
#define OLED_REFRESH_INTERVAL 50    // ??50ms,?????????

uint32_t last_oled_tick = 0;

#define BT_SEND_INTERVAL  100
uint32_t last_bt_tick = 0;
uint16_t last_bt_count = 0xFFFF;
uint16_t last_bt_alarm = 0xFFFF;
uint8_t  last_bt_state = 0xFF;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM2_Init();
    MX_USART1_UART_Init();
    
    OLED_Init();
    SEG_Init();
    
    // ????
    SEG_ShowNumber(1234);
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_Delay(5000);
    
    item_count = 0;
    SEG_ShowNumber(0);
    HAL_Delay(500);
    
    // ????????(????????,??????)
    uint8_t initial_sensor = HAL_GPIO_ReadPin(LIGHT_SENSOR_GPIO_Port, LIGHT_SENSOR_Pin);
    last_sensor = initial_sensor;
    stable_sensor = initial_sensor;
    prev_stable = initial_sensor;
    
    OLED_Clear();
    OLED_ShowString(0, 0, "ITEM COUNTER");
    OLED_ShowString(0, 2, "COUNT:");
    OLED_ShowNum(48, 2, item_count, 4);
    OLED_ShowString(0, 4, "ALARM:");
    OLED_ShowNum(48, 4, alarm_count, 4);
    OLED_Refresh();
    
    while(1)
    {
        // === ???????(????,???)===
        SEG_ShowNumber(item_count);
        
        // === LED ?? ===
        if(alarm_flag)
        {
            HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
        }
        else
        {
            HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        }
        
        // === OLED ??(????????????!)===
        if(oled_refresh_flag && (HAL_GetTick() - last_oled_tick > OLED_REFRESH_INTERVAL))
        {
            oled_refresh_flag = 0;
            last_oled_tick = HAL_GetTick();
            
            if(alarm_flag)
            {
                OLED_Clear();
                OLED_ShowString(0, 0, "!!! ALARM !!!");
                OLED_ShowString(0, 2, "BLOCKED!");
                OLED_ShowNum(0, 4, block_timer, 4);
                OLED_Refresh();
            }
            else
            {
                if(last_alarm_flag != alarm_flag)
                {
                    OLED_Clear();
                    OLED_ShowString(0, 0, "ITEM COUNTER");
                    OLED_ShowString(0, 2, "COUNT:");
                    OLED_ShowString(0, 4, "ALARM:");
                    OLED_Refresh();
                }
                else
                {
                    OLED_ShowString(48, 2, "    ");
                    OLED_ShowNum(48, 2, item_count, 4);
                    OLED_ShowString(48, 4, "    ");
                    OLED_ShowNum(48, 4, alarm_count, 4);
                    OLED_Refresh();
                }
            }
            
            last_alarm_flag = alarm_flag;
        }
        
        // === ???? ===
        if((item_count != last_bt_count || 
            alarm_count != last_bt_alarm || 
            alarm_flag != last_bt_state) &&
           (HAL_GetTick() - last_bt_tick > BT_SEND_INTERVAL))
        {
            last_bt_tick = HAL_GetTick();
            BT_SendData(item_count, alarm_count, alarm_flag);
            last_bt_count = item_count;
            last_bt_alarm = alarm_count;
            last_bt_state = alarm_flag;
        }
        
        // ?????????,?????????????
        HAL_Delay(1);
    }
}
