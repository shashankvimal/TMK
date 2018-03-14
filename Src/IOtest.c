/*****************************************************************************
 *   iotest.c:  main C entry file for NXP LPC17xx Family Microprocessors
 *
 *
 *
******************************************************************************/

#include "lpc17xx.h"
#include "type.h"
#include "uart.h"
#include "spi.h"
#include "timer.h"
#include <utils.h>


void user_waitover(void *pSpinOverVar);

int main (void)
{	    		
	uint32_t i, j, status = 0;
	volatile uint32_t waitOver = 0;
	uint8_t bootBanner[] = "WELCOME LPC 1768 BOOTED\n";
	uint8_t sPass[] = "Status:Pass\n";
	uint8_t sFail[] = "Status:Fail\n";
	TIMER_OBJ_t* pTimer;

	status += system_init();

	status += timer_init();
	
	status += uart0_init();

	status += spi_init(SPI0_MODE_MASTER);


	uart0_tx(bootBanner, sizeof(bootBanner));

	if(BOOT_CHECK_STATE(status) == TRUE)   uart0_tx(sPass, sizeof(sPass));
	else  uart0_tx(sFail, sizeof(sFail));

	pTimer = timer_oneshotAcquire(5000000, user_waitover, (void*)&waitOver);
	timer_start(pTimer);

	while(!waitOver);
	timer_release(pTimer);
	uart0_tx("wait over", 10);

	LPC_GPIO2->FIODIR = 0x000000FF;		/* P2.xx defined as Outputs */
	LPC_GPIO2->FIOCLR = 0x000000FF;		/* turn off all the LEDs */

	LPC_GPIO0->FIODIR |= (1 << 19);
	LPC_GPIO0->FIODIR |= (1 << 16);
	LPC_GPIO0->FIOCLR |= (1 << 16);
	LPC_GPIO0->FIOSET |= (1 << 16);
	while(1)
	{
		for(i = 0; i < 8; i++)
		{
			LPC_GPIO2->FIOSET = 1 << i;
			for(j = 1000000; j > 0; j--);
		}
		LPC_GPIO2->FIOCLR = 0x000000FF;
		for(j = 1000000; j > 0; j--);
	
	}
}

void user_waitover(void *pSpinOverVar)
{
	*(uint32_t*)pSpinOverVar = TRUE;	
}






