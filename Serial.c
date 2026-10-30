#include "stm32f10x.h"
#include "Serial.h"
#include <stdio.h>

	#include <stdarg.h>

uint8_t Serial_RxData;
uint8_t Serial_RxFlag;


void Serial_Init(void)

{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* 1. 开启时钟：USART1和GPIOA都连接在APB2总线上。 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* 2. PA9配置为复用推挽输出，由USART1硬件输出发送波形。
     * GPIO_Speed_50MHz是GPIO输出速度，串口波特率在下面单独设置。
     */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* 3. 设置通信规则：9600波特率，8个数据位，1个停止位，无校验。
     * 本工程对应“串口发送”，只开启发送方向，无需配置接收中断。
     */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx ;//串口接收，可以使用查询和中断两种
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个校验位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//配置定时器中断
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

NVIC_InitTypeDef NVIC_InitStructure;
NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
NVIC_Init(&NVIC_InitStructure);
		
		
		
    /* 4. 使能USART1，完成初始化。 */
    USART_Cmd(USART1, ENABLE);
}

void Serial_SendByte(uint8_t Byte)
{
    USART_SendData(USART1, Byte);

    /* TXE置1表示数据已移入发送移位寄存器，可以准备下一个字节。
     * 此时最后一个字节仍可能在线上发送；判断整帧结束应查看TC。
     */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    {
    }
}

void Serial_SendArray(uint8_t *Array , uint16_t Length) //这是一个遍历数组的函数，目的是为了输出多数据
{
	uint16_t i;
	for (i=0; i<Length; i++)
	{
			Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i ++)
    {
        Serial_SendByte(String[i]);
    }
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y --)
    {
        Result *= X;
    }
    return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i ++)
    {
        Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
    }
}  //这里几个模块就是发送字节，发送数组，发送字符串，发送字符形式 


int fputc(int ch, FILE *f)  //fputc是printf函数的底层，目的就是移植printf
{
    Serial_SendByte(ch);
    return ch;
}

void Serial_Printf(char *format, ...)
{
    char String[100];// 先定义输出的字符串
    va_list arg;//定义参数列表变量  va_list类型名

    va_start(arg, format);//从format位置开始接收参数表，放在arg里面
    vsprintf(String, format, arg);//String打印位置
    va_end(arg);//释放参数表
    Serial_SendString(String);//发送
}

uint8_t Serial_GetRxFlag(void)
{
    if (Serial_RxFlag == 1)
    {
        Serial_RxFlag = 0;
        return 1;
    }
    return 0;
}

uint8_t Serial_GetRxData(void)
{
    return Serial_RxData;
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        Serial_RxData = USART_ReceiveData(USART1);
        Serial_RxFlag = 1;
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}