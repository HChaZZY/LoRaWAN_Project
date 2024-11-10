/**
******************************************************************************
* File Name          : USART.h
* Description        : This file provides code for the configuration
*                      of the USART instances.
******************************************************************************
*
* COPYRIGHT(c) 2017 STMicroelectronics
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usart_H
#define __usart_H
#ifdef __cplusplus
extern "C" {
#endif

    /* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"


    /*-------------���ݾ����Ӳ��ƽ̨����д���º궨���в����ľ���ʵ��---------------*/

    /** ��ģ�������ӵĴ��ڷ��ͺ�������������ָ���ĳ��ȷ��ʹ������� */
#define UART_TO_MODULE_WRITE_DATA(buffer, size)		lpusart1_send_data(buffer, size)

    /** ��ģ�������ӵĴ��ڷ��ͺ�������ָ�����ȣ���\0���� */
#define UART_TO_LRM_WRITE_STRING(buffer)			lpusart1_send_string(buffer)

    /** ��ģ�������ӵĴ��ڽ��ջ��壬Ŀǰ���ڽ������ݽ�֧���жϼ�BUFFER����ʽ����֧��FIFO��ʽ */
#define UART_TO_LRM_RECEIVE_BUFFER					LPUsart1_RX.RX_Buf

    /** ��ģ�������ӵĴ��ڽ��յ����ݵı��� */
#define UART_TO_LRM_RECEIVE_FLAG					LPUsart1_RX.receive_flag

    /** ��ģ�������ӵĴ��ڱ��ν��յ����ݳ��� */
#define UART_TO_LRM_RECEIVE_LENGTH					LPUsart1_RX.rx_len

    /*==========================END OF LPUART1 CONFIG================================*/

    /** ��ģ�������ӵĴ��ڷ��ͺ�������������ָ���ĳ��ȷ��ʹ������� */
#define UART_TO_PC_WRITE_DATA(buffer, size)			usart2_send_data(buffer, size)

    /** ��ģ�������ӵĴ��ڷ��ͺ�������ָ�����ȣ���\0���� */
#define UART_TO_PC_WRITE_STRING(buffer)				usart2_send_string(buffer)

    /** ��ģ�������ӵĴ��ڽ��ջ��壬Ŀǰ���ڽ������ݽ�֧���жϼ�BUFFER����ʽ����֧��FIFO��ʽ */
#define UART_TO_PC_RECEIVE_BUFFER					Usart2_RX.RX_Buf

    /** ��ģ�������ӵĴ��ڽ��յ����ݵı��� */
#define UART_TO_PC_RECEIVE_FLAG						Usart2_RX.receive_flag

    /** ��ģ�������ӵĴ��ڱ��ν��յ����ݳ��� */
#define UART_TO_PC_RECEIVE_LENGTH					Usart2_RX.rx_len

    /*==========================END OF USART2 CONFIG================================*/


    /* USER CODE BEGIN Private defines */
#define RECEIVELEN 1024
#define USART_DMA_SENDING 1
#define USART_DMA_SENDOVER 0

    typedef struct
    {
        uint8_t receive_flag;
        uint8_t dmaSend_flag;
        uint16_t rx_len;
        uint8_t RX_Buf[RECEIVELEN];
    } usart_recv_t;

    extern usart_recv_t Usart1_RX;
    extern usart_recv_t Usart2_RX;
    extern usart_recv_t LPUsart1_RX;

    /* USER CODE END Private defines */


    void MX_LPUART1_Init(uint32_t baudrate);
    void MX_USART2_Init(uint32_t baudrate);

    void MX_USART1_UART_Init(uint32_t baudrate);

    /* USER CODE BEGIN Prototypes */

    void usart2_send_byte(uint8_t data);
    void usart2_send_data(uint8_t *pdata, uint16_t length);
    void usart2_send_string(uint8_t *str);
    void usart2_send_onenumber(uint8_t data);
    void usart2_send_numbers(uint8_t data);
    void USART2_Clear_IT(void);
    void usart2_receive_idle(void);

    void lpusart1_send_byte(uint8_t data);
    void lpusart1_send_data(uint8_t *pdata, uint16_t length);
    void lpusart1_send_string(uint8_t *pdata);
    void lpusart1_receive_idle(void);
    void LPUART1_Clear_IT(void);

    void lpuart1_reconfig(uint32_t baudrate);
    void PrepareLPUARTToStopMode(void);


    void debug_printf(char *fmt, ...);
    /* USER CODE END Prototypes */

    void Usart1Receive_IDLE(void);
    void Usart1SendData(uint8_t *pdata, uint16_t Length);
    void UART1_SendByte(uint8_t data);
    void USART1_SendString(uint8_t *str);
    void Clear_UART1_IT(void);



#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/