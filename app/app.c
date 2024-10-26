#include <string.h>
#include "app.h"
#include "usart.h"
#include "gpio.h"
#include "lorawan_node_driver.h"
#include "hdc1000.h"
#include "sensors_test.h"
#include "ST7789v.h"
#include "XPT2046.h"
#include "opt3001.h"
#include "MPL3115.h"
#include "rtc.h"

#define DEBUG 1
#define DBGPrint \
    if (DEBUG)   \
    debug_printf

extern DEVICE_MODE_T device_mode;
extern DEVICE_MODE_T *Device_Mode_str;

down_list_t *pphead = NULL;
void LoRaWAN_Func_Process(void);
uint8_t joinNetwork = 0;

void DisplayStatus(void);
void SyncRTCTime(void);
//-----------------Users application--------------------------
void LoRaWAN_Func_Process(void)
{
    /* 模块入网判断 */
    if (!joinNetwork)
    {
        joinNetwork = 1;
        DBGPrint("Start Join!\n");
        if (nodeJoinNet(JOIN_TIME_120_SEC) == false)
        {
            DBGPrint("Join Failed!\r\n");
            return;
        }
    }

    SyncRTCTime();

    /* 你的实验代码位置 */
    uint16_t temp, humi;
    float pressure;
    float lux;
    RTC_TimeTypeDef curTime;
    RTC_DateTypeDef curDate;

    temp = HDC1000_Read_Temper();
    humi = HDC1000_Read_Humidi();
    pressure = MPL3115_ReadPressure();
    lux = OPT3001_Get_Lux();

    DBGPrint("\nTemperature: %d.%d\r\n", temp / 1000, temp % 1000);
    DBGPrint("Humidity: %d.%d\r\n", humi / 1000, humi % 1000);
    DBGPrint("Pressure: %.2f\r\n", pressure);
    DBGPrint("Lux: %.2f\r\n", lux);

    HAL_GPIO_WritePin(LedGpio_D7, LedPin_D7, GPIO_PIN_RESET); // 红色LED灯亮，表示风扇不工作
    HAL_GPIO_WritePin(LedGpio_D8, LedPin_D8, GPIO_PIN_RESET); // 绿色LED灯亮，表示喷雾不工作
    HAL_GPIO_WritePin(LedGpio_D6, LedPin_D6, GPIO_PIN_RESET); // 白色LED灯亮，表示加热器不工作

    if (temp / 1000 < 15)
    {
        HAL_GPIO_WritePin(LedGpio_D6, LedPin_D6, GPIO_PIN_SET); // 白色LED灯灭，表示加热器工作
    }

    if (humi / 1000 < 40)
    {
        HAL_GPIO_WritePin(LedGpio_D8, LedPin_D8, GPIO_PIN_SET); // 绿色LED灯灭，表示喷雾工作
    }

    if (temp / 1000 > 25 || humi / 1000 > 60)
    {
        HAL_GPIO_WritePin(LedGpio_D7, LedPin_D7, GPIO_PIN_SET); // 红色LED灯灭，表示风扇工作
    }

    if (HAL_RTC_GetTime(&hrtc, &curTime, RTC_FORMAT_BIN) == HAL_OK && HAL_RTC_GetDate(&hrtc, &curDate, RTC_FORMAT_BIN) == HAL_OK)
    {
        DBGPrint("Current Time: %02d:%02d:%02d \r\n", curTime.Hours, curTime.Minutes, curTime.Seconds, curTime.SubSeconds, curTime.SecondFraction);
    }

    DisplayStatus();

    HAL_Delay(1000);
}

void SyncRTCTime(void) {
    uint8_t tbuff[25] = {0};
    RTC_TimeTypeDef Ptime;
    RTC_DateTypeDef Pdate;
    DBGPrint("Sync RTC Time...\r\n");
    nodeCmdConfig("AT+TIMESYNC\r\n");
    nodeCmdInqiure("AT+RTC?\r\n", tbuff);
    for (int i = 0, j = 0; i < strlen(tbuff); i++)
    {
        if (tbuff[i] != ' ')
            tm[j++] = tbuff[i];
        if (j == 12)
            break;
    }
    HAL_RTC_GetTime(&hrtc, &Ptime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &Pdate, RTC_FORMAT_BIN);
    Pdate.Year = (uint8_t)tm[0] * 10 + (uint8_t)tm[1] - 16;
    Pdate.Month = (uint8_t)tm[2] * 10 + (uint8_t)tm[3] - 16;
    Pdate.Date = (uint8_t)tm[4] * 10 + (uint8_t)tm[5] - 16;
    Ptime.Hours = (uint8_t)tm[6] * 10 + (uint8_t)tm[7] - 16;
    Ptime.Minutes = (uint8_t)tm[8] * 10 + (uint8_t)tm[9] - 16;
    Ptime.Seconds = (uint8_t)tm[10] * 10 + (uint8_t)tm[11] - 16;
    DBGPrint("Sync RTC Time: %d-%d-%d %d:%d:%d\r\n",Pdate.Year,Pdate.Month,Pdate.Date,Ptime.Hours,Ptime.Minutes,Ptime.Seconds);
    HAL_RTC_SetTime(&hrtc, &Ptime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &Pdate, RTC_FORMAT_BIN);
}

void DisplayStatus(void)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    uint16_t temp, humi;
    float pressure;
    float lux;

    // 读取传感器值
    temp = HDC1000_Read_Temper();
    humi = HDC1000_Read_Humidi();
    pressure = MPL3115_ReadPressure();
    lux = OPT3001_Get_Lux();

    LCD_Fill(0, 0, 239, 36, BLUE);
    char tempStr[20];
    sprintf(tempStr, "Temperature: %d.%d C", temp / 1000, temp % 1000);
    LCD_ShowString(10, 10, tempStr, BLACK);
    lpusart1_send_string((uint8_t *)tempStr); // 发送温度值
    // LCD_Clear(0xbefe);

    LCD_Fill(0, 37, 239, 73, BLUE);
    char humiStr[20];
    sprintf(humiStr, "Humidity: %d.%d %", humi / 1000, humi % 1000);
    LCD_ShowString(10, 47, humiStr, BLACK);
    lpusart1_send_string((uint8_t *)humiStr); // 发送湿度值
    // LCD_Clear(0xbefe);

    LCD_Fill(0, 74, 239, 110, BLUE);
    char pressureStr[20];
    sprintf(pressureStr, "Pressure: %.2f Pa", pressure);
    LCD_ShowString(10, 84, pressureStr, BLACK);
    lpusart1_send_string((uint8_t *)pressureStr); // 发送气压值
    // LCD_Clear(0xbefe);

    LCD_Fill(0, 111, 239, 147, BLUE);
    char luxStr[20];
    sprintf(luxStr, "Lux: %.2f lx", lux);
    LCD_ShowString(10, 121, luxStr, BLACK);
    lpusart1_send_string((uint8_t *)luxStr); // 发送光照强度值
                                             // LCD_Clear(0xbefe);

    // LCD_Clear(0xbefe);
}

void LoRaWAN_Borad_Info_Print(void)
{
    debug_printf("\r\n\r\n");
    PRINT_CODE_VERSION_INFO("%s", CODE_VERSION);
    LEDALL_ON;
    HAL_Delay(100);
    LEDALL_OFF;
}
