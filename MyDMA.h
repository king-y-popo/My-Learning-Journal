#ifndef __MYDMA_H
#define __MYDMA_H

#include "stm32f10x.h"

/* 配置 Size 个字节的转运参数，暂不启动：AddrA 为源，AddrB 为目的。 */
void MyDMA_Init(uint32_t AddrA, uint32_t AddrB, uint16_t Size);

/* 启动一次转运，并等待全部数据复制完成。 */
void MyDMA_Transfer(void);

#endif
