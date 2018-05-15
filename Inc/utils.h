#ifndef UTILS_H__
#define UTILS_H__
#include "type.h"

#define FUNCTIONAL_STATUS	
#define ENABLED				(1)
#define DISABLED			!(ENABLED)
#define TOGGLE(val)			~(val)
#define INLINE              __inline

#define SYSTEM_INIT			(1)
#define TIMER_INIT			(2)
#define UART_INIT			(3)
#define SPI_INIT			(4)
#define BOOT_SET_STATE(status)	if((status)) (status) <<= 1
#define BOOT_CHECK_STATE(status)		((status) == 0x4)

//set bit
#define SETB_REG32(REG_ADR, BIT_POS)		*((volatile u32*)(REG_ADR)) |= (1 << (BIT_POS))
//set new value
#define SETV_REG32(REG_ADR, VAL)			*((volatile u32*)(REG_ADR)) = (VAL)
//Get bit
#define GETB_REG32(REG_ADR, BIT_POS)		((*((volatile u32*)(REG_ADR)) & (1 << (BIT_POS))) >> (BIT_POS))
#define SHIFT(REG_NAME, BIT_NAME)		 	(REG_NAME##_##BIT_NAME)

#endif //UTILS_H__
