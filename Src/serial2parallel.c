#include "lpc17xx.h"
#include "type.h"
#include "spi.h"


#if (FUNCTIONAL_STATUS == ENABLED)
void s2p_txData(uint8_t *pData, uint32_t len)
{
	//Enable 74HC595(Serial to parallel converter)
	LPC_GPIO0->FIOCLR |= (1 << 19);
	
	//Tx data
	spi_tx(pData, len);
	
	//Disable 74HC595(Serial to parallel converter)
	LPC_GPIO0->FIOSET |= (1 << 19);
}
#endif
