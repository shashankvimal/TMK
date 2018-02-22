#include "lpc17xx.h"
#include "type.h"
#include "utils.h"
#include "spi.h"
#include <nvic.h>

#define REG_PCONP_BIT_PCSPI			(8)
#define REG_PCLKSEL0_BIT_PCLK_SPI 	(16)
#define REG_PINSEL0_BIT_SCK		    (30)		
#define REG_PINSEL1_BIT_SSEL  		(0)
#define REG_PINSEL1_BIT_MISO  		(2)
#define REG_PINSEL1_BIT_MOSI  		(4)
#define REG_S0SPCR_BIT_MSTR			(5)
#define REG_S0SPCR_BIT_ENABLE		(2)
#define REG_S0SPCR_BIT_CPOL			(4)

typedef volatile struct 
{
	uint32_t S0SPCR;
	uint32_t S0SPSR;
	uint32_t S0SPDR;
	uint32_t S0SPCCR;
	uint32_t S0SPINT;
}SPI_CONTROL_BLOCK_t;

#define SPI0_CB	((SPI_CONTROL_BLOCK_t*)0x40020000)

uint32_t spi_init(uint32_t mode)
{
	uint32_t divisor = 0x9;
	/*Power up the spi block*/
	LPC_SC->PCONP |= (1 << SHIFT(REG_PCONP,BIT_PCSPI));

	/*Set clock for spi master mode: SCK0(1MHz) = PCLK_SPI(9MHz)/SPCCR0(9) where (PCLK = CCLK/8 = 72MHz/8) */
	LPC_SC->PCLKSEL0 &= ~(0x3 << SHIFT(REG_PCLKSEL0,BIT_PCLK_SPI)); 
	LPC_SC->PCLKSEL0 |=	(0x3 << SHIFT(REG_PCLKSEL0,BIT_PCLK_SPI));
	
	if(mode == SPI0_MODE_MASTER)
	{
		/*Set clock divisor*/
		SPI0_CB->S0SPCCR = 0;
		SPI0_CB->S0SPCCR |= (divisor & 0xFF);
		/*Set as spi0 master*/
		SPI0_CB->S0SPCR	 |=	(1 << SHIFT(REG_S0SPCR,BIT_MSTR));
	}
	/*Set the Pin muxer for: SCK0,SSEL0,MOSI0,MISO0*/ 
	LPC_PINCON->PINSEL0 &= ~(0x3 << SHIFT(REG_PINSEL0,BIT_SCK));
	LPC_PINCON->PINSEL0 |=	(0x3 << SHIFT(REG_PINSEL0,BIT_SCK));

	if(mode == SPI0_MODE_SLAVE)
	{
		LPC_PINCON->PINSEL1 &= ~(0x3 << SHIFT(REG_PINSEL1,BIT_SSEL));
		LPC_PINCON->PINSEL1 |= (0x3 << SHIFT(REG_PINSEL1,BIT_SSEL));
	} 

	LPC_PINCON->PINSEL1 &= ~(0x3 << SHIFT(REG_PINSEL1,BIT_MISO));
	LPC_PINCON->PINSEL1 |=	(0x3 << SHIFT(REG_PINSEL1,BIT_MISO));

	LPC_PINCON->PINSEL1 &= ~(0x3 << SHIFT(REG_PINSEL1,BIT_MOSI));
	LPC_PINCON->PINSEL1 |=	(0x3 << SHIFT(REG_PINSEL1,BIT_MOSI));

	/*Enable SPI for variable size data xfer*/
	//SPI0_CB->S0SPCR	 |=	(1 << SHIFT(REG_S0SPCR,BIT_ENABLE));
	
	/*Change clock polarity as active low*/
	SPI0_CB->S0SPCR	 |=	(1 << SHIFT(REG_S0SPCR,BIT_CPOL));

	return 1;
}

void spi_tx(uint8_t* pData, uint32_t len)
{
	uint32_t scrubBuf;

	while(len-- > 0)
	{
		//LPC_GPIO0->FIOSET |= (0x1 << 16);
		
		SPI0_CB->S0SPDR = (*pData++);

		while((SPI0_CB->S0SPSR & (1 << 7)) == 0);

		scrubBuf = SPI0_CB->S0SPDR;

		//LPC_GPIO0->FIOCLR &= (~(0x1 << 16));

	}
}
