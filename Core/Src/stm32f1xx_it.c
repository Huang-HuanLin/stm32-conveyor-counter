/* USER CODE BEGIN Includes */
#include "main.h"    // ???? GPIO ???????
/* USER CODE END Includes */
#include "segment.h"
/* USER CODE BEGIN PV */
// ?????????(??????)
extern uint8_t last_sensor;
extern uint8_t stable_sensor;
extern uint8_t debounce_cnt;
extern uint16_t block_timer;
extern uint8_t alarm_flag;
extern uint16_t item_count;
extern uint16_t alarm_count;
extern uint8_t prev_stable;
extern uint8_t oled_refresh_flag;

#define BLOCK_THRESHOLD  200
#define DEBOUNCE_THRESHOLD 10
/* USER CODE END PV */

/* USER CODE BEGIN 1 */
/* TIM2 ???? - ?1ms??:????? + ????? */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        // === 1. ???????(??)===
        SEG_Scan();
        
        // === 2. ????????(??,?1ms??)===
        uint8_t sensor = HAL_GPIO_ReadPin(LIGHT_SENSOR_GPIO_Port, LIGHT_SENSOR_Pin);
        
        // ??:???????
        if(sensor == last_sensor)
        {
            if(debounce_cnt < DEBOUNCE_THRESHOLD) 
                debounce_cnt++;
            else 
                stable_sensor = sensor;
        }
        else
        {
            debounce_cnt = 0;
        }
        last_sensor = sensor;
        
        // === 3. ??????(???????)===
        if(stable_sensor)
        {
            // ???:??
            block_timer++;
            
            if(block_timer >= BLOCK_THRESHOLD && !alarm_flag)
            {
                alarm_flag = 1;
                alarm_count++;
                oled_refresh_flag = 1;  // ???????OLED
            }
        }
        else
        {
            // ????:?????(1?0),??+1
            if(prev_stable == 1 && !alarm_flag)
            {
                item_count++;
                if(item_count > 9999) item_count = 9999;
                oled_refresh_flag = 1;
            }
            
            block_timer = 0;
            
            if(alarm_flag)
            {
                alarm_flag = 0;
                oled_refresh_flag = 1;
            }
        }
        
        prev_stable = stable_sensor;
    }
}
/* USER CODE END 1 */