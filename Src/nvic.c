#include <stdint.h>
#include <system_LPC17xx.h>
#include <utils.h>

#define ADR_REG_NVIC_ISER0		(0xE000E100)
#define ADR_REG_NVIC_ISER1 		(0xE000E104)
#define ADR_REG_NVIC_ICER0		(0xE000E180)
#define ADR_REG_NVIC_ICER1 		(0xE000E184)
#define ADR_REG_NVIC_ICPR0		(0xE000E280)
#define ADR_REG_NVIC_ICPR1 		(0xE000E284)
#define ADR_REG_NVIC_IABR0		(0xE000E300)
#define ADR_REG_NVIC_IABR1		(0xE000E304)
#define VAL_CLEAR_ALL_GROUP1INT	(0xFFFFFFFF)
#define VAL_CLEAR_ALL_GROUP2INT	(0x7)
#define FACTOR4SANITY_GROUP2INT	(0x7)

typedef enum
{	
	START_OF_GROUP1INT = 0x0,
	/*0*/BIT_POS_WDT = START_OF_GROUP1INT,
	/*1*/BIT_POS_TIMER0,
	/*2*/BIT_POS_TIMER1,
	/*3*/BIT_POS_TIMER2,
	/*4*/BIT_POS_TIMER3,
	/*5*/BIT_POS_UART0,
	/*6*/BIT_POS_UART1,
	/*7*/BIT_POS_UART2,
	/*8*/BIT_POS_UART3,
	/*9*/BIT_POS_PWM,
	/*10*/BIT_POS_I2C0,
	/*11*/BIT_POS_I2C1,
	/*12*/BIT_POS_I2C2,
	/*13*/BIT_POS_SPI,
	/*14*/BIT_POS_SSP0,
	/*15*/BIT_POS_SSP1,
	/*16*/BIT_POS_PLL0,
	/*17*/BIT_POS_RTC,
	/*18*/BIT_POS_EINT0,
	/*19*/BIT_POS_EINT1,
	/*20*/BIT_POS_EINT2,
	/*21*/BIT_POS_EINT3,
	/*22*/BIT_POS_ADC,
	/*23*/BIT_POS_BOD,
	/*24*/BIT_POS_USB,
	/*25*/BIT_POS_CAN,
	/*26*/BIT_POS_DMA,
	/*27*/BIT_POS_I2S,
	/*28*/BIT_POS_ENET,
	/*29*/BIT_POS_RIT,
	/*30*/BIT_POS_MCPWM,
	/*31*/BIT_POS_QEI,
	START_OF_GROUP2INT = (BIT_POS_QEI + 1),
	/*32*/BIT_POS_PLL1 = START_OF_GROUP2INT,
	/*33*/BIT_POS_USBACT,
	/*34*/BIT_POS_CANACT,
	END_OF_GROUP2INT 
}BIT_POS_INT;


__asm void _enable_irq(void)
{
	CPSIE I;
}
__asm void _disable_irq(void)
{
	CPSID I;
}

__asm uint32_t nvic_getCurExceptionNum(void)
{
	MRS R0,IPSR
	BX  LR
} 
void nvic_enableIRQ(uint32_t intId)
{
#if 0
	_disable_irq();
#endif
    if(intId < START_OF_GROUP2INT)
	{
		SETB_REG32(ADR_REG_NVIC_ISER0, intId);
	}
	else if(intId < END_OF_GROUP2INT)
	{
	   SETB_REG32(ADR_REG_NVIC_ISER1, ((intId - START_OF_GROUP2INT) & FACTOR4SANITY_GROUP2INT));
	}
#if 0
	_enable_irq();
#endif
}
void nvic_disableIRQ(uint32_t intId)
{
	_disable_irq();
	if(intId < START_OF_GROUP2INT)
	{
		SETB_REG32(ADR_REG_NVIC_ICER0, intId);
	}
	else if(intId < END_OF_GROUP2INT)
	{
		SETB_REG32(ADR_REG_NVIC_ICER1, ((intId - START_OF_GROUP2INT) & FACTOR4SANITY_GROUP2INT));	
	}
	_enable_irq();
}

void nvic_clearPendingIRQ(uint32_t intId)
{
	if(intId < START_OF_GROUP2INT)
	{
		SETB_REG32(ADR_REG_NVIC_ICPR0, intId);
	}
	else if(intId < END_OF_GROUP2INT)
	{
		SETB_REG32(ADR_REG_NVIC_ICPR1, ((intId - START_OF_GROUP2INT) & FACTOR4SANITY_GROUP2INT));	
	}
}

int32_t nvic_isActiveIRQ(uint32_t intId)
{
	if(intId < START_OF_GROUP2INT)
	{
		return GETB_REG32(ADR_REG_NVIC_IABR0, intId);
	}
	else if(intId < END_OF_GROUP2INT)
	{
		return GETB_REG32(ADR_REG_NVIC_IABR1, ((intId - START_OF_GROUP2INT) & FACTOR4SANITY_GROUP2INT));	
	}
	return -1;
}
void nvic_scrubPendingIRQ(void)
{
	SETV_REG32(ADR_REG_NVIC_ICPR0, VAL_CLEAR_ALL_GROUP1INT);
	SETV_REG32(ADR_REG_NVIC_ICPR1, VAL_CLEAR_ALL_GROUP2INT);
}



