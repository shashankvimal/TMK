#ifndef TIMER_H__
#define TIMER_H__

#define MODE_TIMER		(1)
#define MODE_COUNTER	!(MODE_TIMER)

typedef volatile struct timer
{
	uint32_t IR; 	//Interrupt register
	uint32_t TCR;	//Timer Control register
	uint32_t TC;	//Timer counter
	uint32_t PR; 	//Prescale register
	uint32_t PC; 	//Presacle counter
	uint32_t MCR; 	//Match control register
	uint32_t MR[4];	//Match Register0-3
	uint32_t CCR;	//Capture control Register
	uint32_t CR[2];	//Capture register
	uint32_t EMR;	//External Match Register
	uint32_t CTCR;	//Count control register
}TIMER_t;
typedef volatile struct timer_obj
{
	uint32_t state;
	void*	pArg;
	uint32_t timeoutVal;
	void (*timer_expiryHandler)(void *pArg) ;
	TIMER_t	*pHw;
}TIMER_OBJ_t;

extern uint32_t timer_init(void);
extern uint32_t timer_resume(TIMER_OBJ_t* pTimer);
extern uint32_t timer_start(TIMER_OBJ_t* pTimer);
extern uint32_t timer_stop(TIMER_OBJ_t* pTimer);
extern uint32_t timer_release(TIMER_OBJ_t* pTimer);
extern void timer_isr(void);
extern TIMER_OBJ_t*  timer_oneshotAcquire(	uint32_t timeoutValue, 
									void (*timer_expiryHandler)(void *pArg),
									void *pArg);

#endif //TIMER_H__
