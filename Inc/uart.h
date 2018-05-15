#ifndef __UART_H__
#define __UART_H__

extern uint32_t uart0_init(void);
extern uint32_t uart0_tx(uint8_t* pData, uint32_t len);
extern uint32_t uart0_rx(uint8_t* pData, uint32_t len);

#endif /*END_OF_UART_H_*/
