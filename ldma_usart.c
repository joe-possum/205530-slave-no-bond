#include "em_usart.h"
#include "em_ldma.h"
#include "em_cmu.h"

#define BSP_USART1_CTS_PIN                            (10U)
#define BSP_USART1_CTS_PORT                           (gpioPortC)
#define BSP_USART1_CTS_LOC                            (11U)

#define BSP_USART1_RTS_PIN                            (11U)
#define BSP_USART1_RTS_PORT                           (gpioPortC)
#define BSP_USART1_RTS_LOC                            (11U)

LDMA_TransferCfg_t transfer = LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART1_TXEMPTY);
LDMA_Descriptor_t descriptor = LDMA_DESCRIPTOR_SINGLE_P2P_BYTE(0,&USART1->TXDATA,2048);

void init_usart_ldma (uint8_t *data) {
	CMU_ClockEnable(cmuClock_USART1,1);
	CMU_ClockEnable(cmuClock_GPIO,1);
	USART_InitAsync_TypeDef uinit = USART_INITASYNC_DEFAULT;
	USART_InitAsync(USART1, &uinit);
	GPIO_PinModeSet(gpioPortC,6,gpioModePushPull,1);
	USART1->ROUTELOC0 = 11 << 8;
	USART1->ROUTEPEN = 2;
#ifdef DEBUG
	USART_Tx(USART1,'H');
	USART_Tx(USART1,'e');
	USART_Tx(USART1,'l');
	USART_Tx(USART1,'l');
	USART_Tx(USART1,'o');
	USART_Tx(USART1,data[0]);
	USART_Tx(USART1,'\n');
#endif
	LDMA_Init_t linit = LDMA_INIT_DEFAULT;
	LDMA_Init(&linit);
	descriptor.xfer.srcAddr = (uint32_t)data;
	LDMA_StartTransfer(0,&transfer,&descriptor);
}

char buf[16];
void LDMA_IRQHandler(void) {
	uint32_t flags = LDMA_IntGet();
	LDMA_IntClear(flags);
#ifdef DEBUG
	int c = sprintf(buf,"LDMA->IF: %08lx\n",flags);
	for(int i = 0; i < c; i++) USART_Tx(USART1,buf[i]);
#else
	LDMA_StartTransfer(0,&transfer,&descriptor);
#endif
}
