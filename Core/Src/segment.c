#include "segment.h"
#include "gpio.h"

// 如果 gpio.h 没有定义，手动添加这些宏
#ifndef DIG_1_GPIO_Port
#define DIG_1_Pin        GPIO_PIN_10
#define DIG_1_GPIO_Port  GPIOB
#define DIG_2_Pin        GPIO_PIN_11
#define DIG_2_GPIO_Port  GPIOB
#define DIG_3_Pin        GPIO_PIN_12
#define DIG_3_GPIO_Port  GPIOB
#define DIG_4_Pin        GPIO_PIN_13
#define DIG_4_GPIO_Port  GPIOB

#define SEG_A_Pin        GPIO_PIN_0
#define SEG_A_GPIO_Port  GPIOB
#define SEG_B_Pin        GPIO_PIN_1
#define SEG_B_GPIO_Port  GPIOB
#define SEG_C_Pin        GPIO_PIN_3
#define SEG_C_GPIO_Port  GPIOB
#define SEG_D_Pin        GPIO_PIN_4
#define SEG_D_GPIO_Port  GPIOB
#define SEG_E_Pin        GPIO_PIN_5
#define SEG_E_GPIO_Port  GPIOB
#define SEG_F_Pin        GPIO_PIN_8
#define SEG_F_GPIO_Port  GPIOB
#define SEG_G_Pin        GPIO_PIN_9
#define SEG_G_GPIO_Port  GPIOB
#define SEG_DP_Pin       GPIO_PIN_4
#define SEG_DP_GPIO_Port GPIOA
#endif

// 共阳数码管段码（0=亮，1=灭）
// 段序：dp,g,f,e,d,c,b,a
static const uint8_t SEG_CODE[] = {
    0xC0, // 0: 1100 0000
    0xF9, // 1: 1111 1001
    0xA4, // 2: 1010 0100
    0xB0, // 3: 1011 0000
    0x99, // 4: 1001 1001
    0x92, // 5: 1001 0010
    0x82, // 6: 1000 0010
    0xF8, // 7: 1111 1000
    0x80, // 8: 1000 0000
    0x90, // 9: 1001 0000
    0x88, // A: 1000 1000
    0xC7, // L: 1100 0111
    0xFF, // 空格: 全灭
    0xBF, // -: 1011 1111
};

static uint8_t disp_buf[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t scan_pos = 0;

// 段选输出
void SEG_WriteSegData(uint8_t data)
{
    HAL_GPIO_WritePin(SEG_A_GPIO_Port, SEG_A_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_B_GPIO_Port, SEG_B_Pin, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_C_GPIO_Port, SEG_C_Pin, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_D_GPIO_Port, SEG_D_Pin, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_E_GPIO_Port, SEG_E_Pin, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_F_GPIO_Port, SEG_F_Pin, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_G_GPIO_Port, SEG_G_Pin, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_DP_GPIO_Port, SEG_DP_Pin, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 动态扫描
void SEG_Scan(void)
{
    // 1. 关闭所有位选
    HAL_GPIO_WritePin(DIG_1_GPIO_Port, DIG_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG_2_GPIO_Port, DIG_2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG_3_GPIO_Port, DIG_3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG_4_GPIO_Port, DIG_4_Pin, GPIO_PIN_RESET);
    
    // 2. 段选全灭（消影）
    HAL_GPIO_WritePin(SEG_A_GPIO_Port, SEG_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_B_GPIO_Port, SEG_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_C_GPIO_Port, SEG_C_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_D_GPIO_Port, SEG_D_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_E_GPIO_Port, SEG_E_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_F_GPIO_Port, SEG_F_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_G_GPIO_Port, SEG_G_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_DP_GPIO_Port, SEG_DP_Pin, GPIO_PIN_SET);
    
    // 3. 消影延时（约50us）
    for(volatile uint8_t j = 0; j < 50; j++);
    
    // 4. 输出段码
    SEG_WriteSegData(disp_buf[scan_pos]);
    
    // 5. 选通当前位（关键修复：scan_pos与DIG反着对应）
    // 硬件实际：DIG_1在最右边，DIG_4在最左边
    // 所以 scan_pos=0(个位) 要送到最右边(DIG_1)
    switch(scan_pos)
    {
        case 0: HAL_GPIO_WritePin(DIG_1_GPIO_Port, DIG_1_Pin, GPIO_PIN_SET); break;  // 个位→最右边
        case 1: HAL_GPIO_WritePin(DIG_2_GPIO_Port, DIG_2_Pin, GPIO_PIN_SET); break;  // 十位
        case 2: HAL_GPIO_WritePin(DIG_3_GPIO_Port, DIG_3_Pin, GPIO_PIN_SET); break;  // 百位
        case 3: HAL_GPIO_WritePin(DIG_4_GPIO_Port, DIG_4_Pin, GPIO_PIN_SET); break;  // 千位→最左边
    }
    
    // 6. 显示延时（约1ms）
    for(volatile uint16_t j = 0; j < 4000; j++);
    
    // 7. 切换到下一位置
    scan_pos++;
    if(scan_pos >= 4) scan_pos = 0;
}

// 显示4位数字（关键修复：disp_buf反着存）
// 目标：显示 1234
// disp_buf[0] → scan_pos=0 → DIG_1 → 最右边 → 应该放个位(4)
// disp_buf[3] → scan_pos=3 → DIG_4 → 最左边 → 应该放千位(1)
void SEG_ShowNumber(uint32_t num)
{
    if(num > 9999) num = 9999;
    disp_buf[0] = SEG_CODE[num % 10];           // 个位 → disp_buf[0] → 最右边
    disp_buf[1] = SEG_CODE[(num / 10) % 10];    // 十位 → disp_buf[1]
    disp_buf[2] = SEG_CODE[(num / 100) % 10];   // 百位 → disp_buf[2]
    disp_buf[3] = SEG_CODE[num / 1000];         // 千位 → disp_buf[3] → 最左边
}

// 显示4字符（同样反着存）
void SEG_ShowString(const char *str)
{
    for(uint8_t i = 0; i < 4; i++)
    {
        uint8_t code = SEG_CODE[12]; // 默认空格
        if(str[i] >= '0' && str[i] <= '9')
            code = SEG_CODE[str[i] - '0'];
        else if(str[i] == 'A')
            code = SEG_CODE[10];
        else if(str[i] == 'L')
            code = SEG_CODE[11];
        else if(str[i] == ' ')
            code = SEG_CODE[12];
        else if(str[i] == '-')
            code = SEG_CODE[13];
        
        // 反着存入：str[0]是最左边字符，对应disp_buf[3]
        disp_buf[3 - i] = code;
    }
}

// 初始化
void SEG_Init(void)
{
    for(uint8_t i = 0; i < 4; i++)
        disp_buf[i] = SEG_CODE[12];
    
    // 强制设置位选为高电平（关闭所有位）
    HAL_GPIO_WritePin(DIG_1_GPIO_Port, DIG_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIG_2_GPIO_Port, DIG_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIG_3_GPIO_Port, DIG_3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIG_4_GPIO_Port, DIG_4_Pin, GPIO_PIN_SET);
    
    // 强制设置段选为高电平（全灭）
    HAL_GPIO_WritePin(SEG_A_GPIO_Port, SEG_A_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_B_GPIO_Port, SEG_B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_C_GPIO_Port, SEG_C_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_D_GPIO_Port, SEG_D_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_E_GPIO_Port, SEG_E_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_F_GPIO_Port, SEG_F_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_G_GPIO_Port, SEG_G_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG_DP_GPIO_Port, SEG_DP_Pin, GPIO_PIN_SET);
}