#include "stm32f10x.h"
#include "MyDMA.h"

/* 保存本次配置的数据单元数，供每次启动前重新装载。 */
static uint16_t MyDMA_Size;

/*
 * 第一个实验：配置 DMA1 通道1，源地址为 AddrA，目的地址为 AddrB。
 * 初始化时通道保持关闭，由 MyDMA_Transfer() 启动一次转运。
 * Size 是数据单元的个数；本实验的数据宽度为 Byte，所以 4 表示 4 字节。
 */
void MyDMA_Init(uint32_t AddrA, uint32_t AddrB, uint16_t Size)
{
    DMA_InitTypeDef DMA_InitStructure;

    MyDMA_Size = Size;

    /* DMA 挂在 AHB 总线上，配置通道前先打开 DMA1 时钟。 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//打开时钟’
    DMA_DeInit(DMA1_Channel1);

    /*
     * 内存到内存模式下，“外设地址”寄存器也可以填写内存地址。
     * PeripheralSRC 表示从外设地址端读取，因此这里 AddrA 是源、AddrB 是目的。
     */
    DMA_InitStructure.DMA_PeripheralBaseAddr = AddrA;//首先这个是外设的地址
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//这个是读取的带宽
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;//源地址的递增
    DMA_InitStructure.DMA_MemoryBaseAddr = AddrB;//这个存储器的地址
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//这个是读取的带宽
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//源地址的递增
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//这个是转换的方向
    DMA_InitStructure.DMA_BufferSize = Size;//这个是定义一下倒数计数器
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//这个是选择模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//运送的优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;//软件触发还是硬件触发
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    /*
     * 初始化只配置 DMA，主程序需要转运时再调用 MyDMA_Transfer()。
     * M2M 模式下通道一旦使能就会开始搬运，因此这里保持 DISABLE。
     */
    DMA_Cmd(DMA1_Channel1, DISABLE);
}

/* 从配置的源地址向目的地址转运一次，等待完成后返回。 */
void MyDMA_Transfer(void)
{
    /* 普通模式转运完成后计数器为0，必须关闭通道再重新装载数量。 */
    DMA_Cmd(DMA1_Channel1, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel1, MyDMA_Size);
    DMA_ClearFlag(DMA1_FLAG_GL1);

    DMA_Cmd(DMA1_Channel1, ENABLE);

    while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET)
    {
    }

    DMA_ClearFlag(DMA1_FLAG_TC1);
}
