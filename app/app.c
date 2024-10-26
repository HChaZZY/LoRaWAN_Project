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

extern DEVICE_MODE_T device_mode;
extern DEVICE_MODE_T *Device_Mode_str;

down_list_t *pphead = NULL;
void DisplayStatus();
//-----------------Users application--------------------------
void LoRaWAN_Func_Process(void)
{
    static DEVICE_MODE_T dev_stat = NO_MODE;

    uint16_t temper = 0;


    switch((uint8_t)device_mode)
    {
    /* 指令模式 */
    case CMD_CONFIG_MODE:
    {
        /* 如果不是command Configuration function, 则进入if语句,只执行一次 */
        if(dev_stat != CMD_CONFIG_MODE)
        {
            dev_stat = CMD_CONFIG_MODE;
            debug_printf("\r\n[Command Mode]\r\n");

            nodeGpioConfig(wake, wakeup);
            nodeGpioConfig(mode, command);
        }
        /* 等待usart2产生中断 */
        if(UART_TO_PC_RECEIVE_FLAG)
        {
            UART_TO_PC_RECEIVE_FLAG = 0;
            lpusart1_send_data(UART_TO_PC_RECEIVE_BUFFER,UART_TO_PC_RECEIVE_LENGTH);
        }
        /* 等待lpuart1产生中断 */
        if(UART_TO_LRM_RECEIVE_FLAG)
        {
            UART_TO_LRM_RECEIVE_FLAG = 0;
            usart2_send_data(UART_TO_LRM_RECEIVE_BUFFER,UART_TO_LRM_RECEIVE_LENGTH);
        }
    }
    break;

    /* 透传模式 */
    case DATA_TRANSPORT_MODE:
    {
        /* 如果不是data transport function,则进入if语句,只执行一次 */
        if(dev_stat != DATA_TRANSPORT_MODE)
        {
            dev_stat = DATA_TRANSPORT_MODE;
            debug_printf("\r\n[Transperant Mode]\r\n");

            /* 模块入网判断 */
            if(nodeJoinNet(JOIN_TIME_120_SEC) == false)
            {
                return;
            }

            temper = HDC1000_Read_Temper()/1000;

            nodeDataCommunicate((uint8_t*)&temper,sizeof(temper),&pphead);
        }

        /* 等待usart2产生中断 */
        if(UART_TO_PC_RECEIVE_FLAG && GET_BUSY_LEVEL)  //Ensure BUSY is high before sending data
        {
            UART_TO_PC_RECEIVE_FLAG = 0;
            nodeDataCommunicate((uint8_t*)UART_TO_PC_RECEIVE_BUFFER, UART_TO_PC_RECEIVE_LENGTH, &pphead);
        }

        /* 如果模块正忙, 则发送数据无效，并给出警告信息 */
        else if(UART_TO_PC_RECEIVE_FLAG && (GET_BUSY_LEVEL == 0))
        {
            UART_TO_PC_RECEIVE_FLAG = 0;
            debug_printf("--> Warning: Don't send data now! Module is busy!\r\n");
        }

        /* 等待lpuart1产生中断 */
        if(UART_TO_LRM_RECEIVE_FLAG)
        {
            UART_TO_LRM_RECEIVE_FLAG = 0;
            usart2_send_data(UART_TO_LRM_RECEIVE_BUFFER,UART_TO_LRM_RECEIVE_LENGTH);
        }
    }
    break;

    /*工程模式*/
    case PRO_TRAINING_MODE:
    {
        /* 如果不是Class C云平台数据采集模式, 则进入if语句,只执行一次 */
        if(dev_stat != PRO_TRAINING_MODE) {
            dev_stat = PRO_TRAINING_MODE;
            debug_printf("\r\n[Project Mode]\r\n");
        }
        /* 你的实验代码位置 */
        uint16_t temp, humi;
        float pressure;
        float lux;

        temp = HDC1000_Read_Temper();
        humi = HDC1000_Read_Humidi();
        pressure = MPL3115_ReadPressure();
        lux = OPT3001_Get_Lux();


        debug_printf("\nTemperature: %d.%d\r\n",temp/1000,temp%1000);
        debug_printf("Humidity: %d.%d\r\n",humi/1000,humi%1000);
        debug_printf("Pressure: %.2f\r\n",pressure);
        debug_printf("Lux: %.2f\r\n",lux);

        HAL_GPIO_WritePin(LedGpio_D7, LedPin_D7, GPIO_PIN_RESET); // 红色LED灯亮，表示风扇不工作
        HAL_GPIO_WritePin(LedGpio_D8, LedPin_D8, GPIO_PIN_RESET); // 绿色LED灯亮，表示喷雾不工作
        HAL_GPIO_WritePin(LedGpio_D6, LedPin_D6, GPIO_PIN_RESET); // 白色LED灯亮，表示加热器不工作

        if (temp / 1000 < 15) {
            HAL_GPIO_WritePin(LedGpio_D6, LedPin_D6, GPIO_PIN_SET); // 白色LED灯灭，表示加热器工作
        }

        if (humi / 1000 < 40) {
            HAL_GPIO_WritePin(LedGpio_D8, LedPin_D8, GPIO_PIN_SET); // 绿色LED灯灭，表示喷雾工作
        }

        if (temp / 1000 > 25 || humi / 1000 > 60) {
            HAL_GPIO_WritePin(LedGpio_D7, LedPin_D7, GPIO_PIN_SET); // 红色LED灯灭，表示风扇工作
        }

        RTC_TimeTypeDef curTime;
        RTC_DateTypeDef curDate;

        if (HAL_RTC_GetTime(&hrtc, &curTime, RTC_FORMAT_BIN) == HAL_OK && HAL_RTC_GetDate(&hrtc, &curDate, RTC_FORMAT_BIN) == HAL_OK){
            debug_printf("Current Time: %02d:%02d:%02d \r\n",curTime.Hours,curTime.Minutes,curTime.Seconds,curTime.SubSeconds,curTime.SecondFraction);
        }

        // HAL_Delay(1000);

    }
    break;

    default:
        break;
    }
}


void LoRaWAN_Borad_Info_Print(void)
{
    debug_printf("\r\n\r\n");
    PRINT_CODE_VERSION_INFO("%s",CODE_VERSION);
    debug_printf("\r\n");
    debug_printf("--> Press Key1 to: \r\n");
    debug_printf("-->  - Enter command Mode\r\n");
    debug_printf("-->  - Enter Transparent Mode\r\n");
    debug_printf("--> Press Key2 to: \r\n");
    debug_printf("-->  - Enter Project Trainning Mode\r\n");
    LEDALL_ON;
    HAL_Delay(100);
    LEDALL_OFF;
}



