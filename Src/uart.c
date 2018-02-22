#include <stdint.h>
#include <type.h>
#include <lpc17xx.h>
#include <nvic.h>

extern uint32_t SystemFrequency;
extern uint8_t 	 g_FrameBuffer[];
extern uint8_t 	g_FrameIdx;


uint32_t uart0_init(void)
{
	uint32_t PCLK =  (SystemFrequency / 4);
	const uint32_t baudrate = 9600;
	uint32_t brfactor = ((PCLK / 16) / baudrate);
	uint32_t DLM = (brfactor / 256);
	uint32_t DLL = (brfactor % 256); 

	/* UART0 is powered up by default on POR; UART0 CLK = CCLK/4 = 18MHz*/
	LPC_SC->PCLKSEL0 &= ~((uint32_t)(3 << 6));
	
	/* Enable divisor latches : DLAB = 1*/
	LPC_UART0->LCR |= (1 << 7);
	/*Set DLM*/
	LPC_UART0->DLM = (DLM & 0xFF);
	/*Set DLL */
	LPC_UART0->DLL = (DLL & 0xFF);

	/* Disable divisor latches : DLAB = 0*/
	LPC_UART0->LCR &= ~(1 << 7);

	/* Enable and clear RX-TX FIFO*/
	LPC_UART0->FCR = 7;

	/*Enable Rx INT: RBR INT*/
	LPC_UART0->IER |= 0x1;
	nvic_enableIRQ(UART0_IRQn);

	/* Pin select for UART Rx-Tx*/
	LPC_PINCON->PINSEL0 |= ((1 << 4) | (1 << 6));

	/* set UART frame structure: 8-bit character, 1 stop bit, no parity*/
	LPC_UART0->LCR |= 0x3;

	return 1;

}

uint32_t uart0_tx(uint8_t* pData, uint32_t len)
{
	while(len-- > 0)
	{
		LPC_UART0->THR = *pData++;

		while(!((LPC_UART0->LSR & (1 << 5)) >> 5));
	}
	return TRUE;
}
uint32_t uart0_rx(uint8_t* pData, uint32_t len)
{
	return TRUE;
}


void uart0_isr(void)
{
 #if 0
	  if(!((LPC_UART0->LSR >> 7) == 0x1))
	  {
		  if(((LPC_UART0->IIR >> 1) & 0x2 == 0x2))
		  {
		  	g_FrameBuffer[g_FrameIdx++]	= LPC_UART0->RBR;
	
			if(g_FrameIdx >= 256)
			{
				g_FrameIdx = 0;
			}
		  }
	  }
	  nvic_scrubPendingIRQ();
#endif
}
