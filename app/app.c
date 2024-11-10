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
char receive[20] = "";

void DisplayStatus(void);
void SyncRTCTime(void);
void BackupStrategy(void);
void ParseReceiveString(void);
void LoRaWAN_Init(void);
void print_business_data(down_list_t **list_head);

#define MAX_RECORDS 48 // ??48?????30??????24??
typedef struct {
    uint16_t temp;
    uint16_t humi;
    float pressure;
    RTC_TimeTypeDef rtc_time;
    uint8_t rain_flag; // ?????1?????0?????
} SensorData;

SensorData records[MAX_RECORDS];
uint8_t record_index = 0;

typedef struct {
    uint16_t temp;
    uint16_t humi;
    float pressure;
    int count;
} TempData;

TempData tempDataBuffer = {0};

RTC_TimeTypeDef firstRecordTime;
RTC_DateTypeDef firstRecordDate;
struct sensors_data_t
{
    uint16_t temp, humi;
    float pressure;
};

//-----------------Users application--------------------------

void LoRaWAN_Func_Process(void)
{
    uint16_t temp, humi;
    float pressure;
    float lux;
    RTC_TimeTypeDef curTime;
    RTC_DateTypeDef curDate;
    static uint8_t last_record_index = 0;
		
    DBGPrint("\n??????\n");
	
    if (!joinNetwork)
    {
        DBGPrint("???????\n");
        joinNetwork = 1;

        if (nodeJoinNet(JOIN_TIME_120_SEC) == false)
        {
            DBGPrint("???????\r\n");
            joinNetwork = 0;
        }
    }

    if (joinNetwork)
    {
        SyncRTCTime();

        if (UART_TO_LRM_RECEIVE_FLAG)
        {
            DBGPrint("??????...\n");
            UART_TO_LRM_RECEIVE_FLAG = 0;
            memset(receive, '\0', sizeof(receive));
            for (int i = 0; i < (LPUsart1_RX.rx_len - ((UART_TO_LRM_RECEIVE_BUFFER[135] >= ' ' && UART_TO_LRM_RECEIVE_BUFFER[135] <= '~') ? 0x0087 : 0x0088)); i++)
            {
                receive[i] = UART_TO_LRM_RECEIVE_BUFFER[i + ((UART_TO_LRM_RECEIVE_BUFFER[135] >= ' ' && UART_TO_LRM_RECEIVE_BUFFER[135] <= '~') ? 135 : 136)];
            }
            ParseReceiveString();
        }

        temp = HDC1000_Read_Temper();
        humi = HDC1000_Read_Humidi();
        pressure = MPL3115_ReadPressure();
        lux = OPT3001_Get_Lux();

        // ?????????????
        tempDataBuffer.temp += temp;
        tempDataBuffer.humi += humi;
        tempDataBuffer.pressure += pressure;
        tempDataBuffer.count++;

        HAL_RTC_GetTime(&hrtc, &curTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &curDate, RTC_FORMAT_BIN);

        if (tempDataBuffer.count == 1)
        {
            firstRecordTime = curTime;
            firstRecordDate = curDate;
        }

        if ((curDate.Date * 24 * 60 + curTime.Hours * 60 + curTime.Minutes) -
            (firstRecordDate.Date * 24 * 60 + firstRecordTime.Hours * 60 + firstRecordTime.Minutes) >= 30)
        {
            // ????????
            records[record_index].temp = tempDataBuffer.temp / tempDataBuffer.count;
            records[record_index].humi = tempDataBuffer.humi / tempDataBuffer.count;
            records[record_index].pressure = tempDataBuffer.pressure / tempDataBuffer.count;
            records[record_index].rtc_time = curTime;

            // ????????
            if (record_index > 0)
            {
                int prev_index = (record_index - 1 + MAX_RECORDS) % MAX_RECORDS;
                int16_t temp_diff = records[prev_index].temp - records[record_index].temp;

                // ???1??????Records???2???????????90%?????100000Pa
                records[record_index].rain_flag = (temp_diff >= 2000 && records[record_index].humi > 90000 && records[record_index].pressure < 100000) ? 1 : 0;
            }
            else
            {
                records[record_index].rain_flag = 0;
            }

            // ??????????
            last_record_index = record_index;

            record_index = (record_index + 1) % MAX_RECORDS; // ??????

            // ?????????
            memset(&tempDataBuffer, 0, sizeof(tempDataBuffer));
        }

        // ????24????????2??????????
        uint8_t rain_count = 0;
        uint8_t continuous_rain = 0;
        for (int i = 0; i < MAX_RECORDS; i++)
        {
            int index = (record_index - 1 - i + MAX_RECORDS) % MAX_RECORDS;
            if (records[index].rain_flag)
            {
                rain_count++;
                if (rain_count >= 4) // 4?????2????30???????
                {
                    continuous_rain = 1;
                    break;
                }
            }
            else
            {
                rain_count = 0; // ??????
            }
        }

        char buffer[1024] = "";
        if (continuous_rain) // ????2????
        {
            sprintf(buffer, "T:%d,%d\nH:%d,%d\nP:%.2f\nL:%.2f\nR:1\n", temp / 1000, temp % 1000, humi / 1000, humi % 1000, pressure / 1000, lux);
        }
        else
        {
            sprintf(buffer, "T:%d,%d\nH:%d,%d\nP:%.2f\nL:%.2f\nR:0\n", temp / 1000, temp % 1000, humi / 1000, humi % 1000, pressure / 1000, lux);
        }

        DBGPrint(buffer);
        nodeDataCommunicate((uint8_t*)&buffer, strlen(buffer), &pphead);
        print_business_data(&pphead);
    }
    BackupStrategy();
    DisplayStatus();
    HAL_Delay(1000);
}

void BackupStrategy(void)
{
    uint16_t temp, humi;
    float pressure;
    float lux;

    temp = HDC1000_Read_Temper();
    humi = HDC1000_Read_Humidi();
    pressure = MPL3115_ReadPressure();
    lux = OPT3001_Get_Lux();

    // TODO: ??????????????????LED????????????????????

    HAL_GPIO_WritePin(LedGpio_D7, LedPin_D7, GPIO_PIN_RESET); // ??LED?????????
    HAL_GPIO_WritePin(LedGpio_D8, LedPin_D8, GPIO_PIN_RESET); // ??LED?????????
    HAL_GPIO_WritePin(LedGpio_D6, LedPin_D6, GPIO_PIN_RESET); // ??LED?????????

    if (temp / 1000 < 15)
    {
        HAL_GPIO_WritePin(LedGpio_D7, LedPin_D7, GPIO_PIN_SET); // ????15??????LED
    }

    if (humi / 1000 < 40)
    {
        HAL_GPIO_WritePin(LedGpio_D8, LedPin_D8, GPIO_PIN_SET); // ????40%?????LED
    }

    if (temp / 1000 > 25 || humi / 1000 > 60)
    {
        HAL_GPIO_WritePin(LedGpio_D6, LedPin_D6, GPIO_PIN_SET); // ????25??????60%?????LED
    }
}

void SyncRTCTime(void)
{
    uint8_t tbuff[25] = {0};
    uint8_t tm[25] = {0};
    RTC_TimeTypeDef Ptime;
    RTC_DateTypeDef Pdate;
    DBGPrint("??RTC??...\r\n");
    nodeCmdConfig("AT+TIMESYNC\r\n");
    nodeCmdInqiure("AT+RTC?\r\n", tbuff);
    for (int i = 0, j = 0; i < strlen(tbuff); i++)
    {
        if (tbuff[i] != ' ')
            tm[j++] = tbuff[i];
        if (j == 12)
            break;
    }
    RTC_TimeTypeDef oldTime;
    HAL_RTC_GetTime(&hrtc, &oldTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &Pdate, RTC_FORMAT_BIN);

    Pdate.Year = (uint8_t)tm[0] * 10 + (uint8_t)tm[1] - 16;
    Pdate.Month = (uint8_t)tm[2] * 10 + (uint8_t)tm[3] - 16;
    Pdate.Date = (uint8_t)tm[4] * 10 + (uint8_t)tm[5] - 16;
    Ptime.Hours = (uint8_t)tm[6] * 10 + (uint8_t)tm[7] - 16;
    Ptime.Minutes = (uint8_t)tm[8] * 10 + (uint8_t)tm[9] - 16;
    Ptime.Seconds = (uint8_t)tm[10] * 10 + (uint8_t)tm[11] - 16;

    // ???????????
    if (Pdate.Year == 24 && Pdate.Month <= 12 && Pdate.Date <= 31 && Ptime.Hours <= 23 && Ptime.Minutes <= 59 && Ptime.Seconds <= 59)
    {
        HAL_RTC_SetTime(&hrtc, &Ptime, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &Pdate, RTC_FORMAT_BIN);
        DBGPrint("RTC??????: %d-%d-%d %d:%d:%d\r\n", Pdate.Year, Pdate.Month, Pdate.Date, Ptime.Hours, Ptime.Minutes, Ptime.Seconds);
    }
    else {
        DBGPrint("RTC?????????: %d-%d-%d %d:%d:%d\r\n", Pdate.Year, Pdate.Month, Pdate.Date, Ptime.Hours, Ptime.Minutes, Ptime.Seconds);
    }
}

void DisplayStatus(void)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    uint16_t temp, humi;
    float pressure;
    float lux;

    // ???????
    temp = HDC1000_Read_Temper();
    humi = HDC1000_Read_Humidi();
    pressure = MPL3115_ReadPressure();
    lux = OPT3001_Get_Lux();

    LCD_Fill(0, 0, 239, 36, BLUE);
    char tempStr[20];
    sprintf(tempStr, "??: %d.%d °C", temp / 1000, temp % 1000);
    LCD_ShowString(10, 10, tempStr, BLACK);
    lpusart1_send_string((uint8_t *)tempStr); // ??????

    LCD_Fill(0, 37, 239, 73, BLUE);
    char humiStr[20];
    sprintf(humiStr, "??: %d.%d %%", humi / 1000, humi % 1000);
    LCD_ShowString(10, 47, humiStr, BLACK);
    lpusart1_send_string((uint8_t *)humiStr); // ??????

    LCD_Fill(0, 74, 239, 110, BLUE);
    char pressureStr[20];
    sprintf(pressureStr, "??: %.2f Pa", pressure);
    LCD_ShowString(10, 84, pressureStr, BLACK);
    lpusart1_send_string((uint8_t *)pressureStr); // ??????

    LCD_Fill(0, 111, 239, 147, BLUE);
    char luxStr[20];
    sprintf(luxStr, "Lux: %.2f lx", lux);
    LCD_ShowString(10, 121, luxStr, BLACK);
    lpusart1_send_string((uint8_t *)luxStr); // ???????????
    // LCD_Clear(0xbefe);

    // LCD_Clear(0xbefe);
}

void ParseReceiveString(void){
	DBGPrint("\nReceived Data: \n%s\n", receive);
}

void LoRaWAN_Init(void){
    nodeCmdConfig("AT+CLASS=2\r\n");
    nodeCmdConfig("AT+RX2=0,505300000\r\n");
    nodeCmdConfig("AT+SAVE\r\n");
    nodeCmdConfig("AT+RESET\r\n");
}

void print_business_data(down_list_t **list_head) {
    down_list_t *current = *list_head;
    const char *target_data = "****CLASSC 1234567890****";
    size_t target_length = strlen(target_data); // ????????
    
    DBGPrint("Print Business Data\n");
    
    while (current != NULL) {
        // ????????
        DBGPrint("Data: %.*s*****************\n", current->down_info.size, current->down_info.business_data);
        
        // ?????????????????
        if (current->down_info.size >= target_length &&
            memcmp(current->down_info.business_data, target_data, target_length) == 0) {
            DBGPrint("Result: VALIDATED\n");
        } else {
            DBGPrint("Result: INVALID\n");
        }
        
        current = current->next;
    }
}



void LoRaWAN_Board_Info_Print(void)
{
    debug_printf("\r\n\r\n");
    PRINT_CODE_VERSION_INFO("%s", CODE_VERSION);
    LEDALL_ON;
    HAL_Delay(100);
    LEDALL_OFF;
}
