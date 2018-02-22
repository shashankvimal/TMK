#include <stdint.h>
#include <type.h>
#include <lpc17xx.h>
#include <timer.h>

#define SYSTICK_TIMER_ADDRESS		(0xE000E010)
#define SYSTICK_TIMER				(SYSTICK_TIMER_t*)SYSTICK_TIMER_ADDRESS
#define NO_OF_TIMERS				(sizeof(timer)/sizeof(timer[0]))
#define PCLK_DIV_BY_FACTOR			(0x9)
#define INT_AND_STOP				(0x5)
#define TIMER_ID(x)					((x) + 1)
#define TIMER0_EXCEPTION_NUM		(0x11)
typedef enum
{
	TIMER0_PWR_ON = (1 << 1),
	TIMER1_PWR_ON = (1 << 2),
	TIMER2_PWR_ON = (1 << 22),
	TIMER3_PWR_ON = (1 << 23),
}TIMER_PWR_CTRL_e;

typedef enum
{
	TIMER0_CLK_EN = (3 << 2),
	TIMER1_CLK_EN = (3 << 4),
	TIMER2_CLK_EN = (3 << 12),
	TIMER3_CLK_EN = (3 << 14),
}TIMER_CLK_CTRL_e;

typedef enum
{
	UNINIT,
	ACQUIRED,
	FREE,
	RUNNING,
	STOPPED,
	EXPIRED
}TIMER_STATE_e;

typedef volatile struct sysTick
{
	uint32_t STCTRL;
	uint32_t STRELOAD;
	uint32_t STCURR;
	uint32_t STCALIB;
}SYSTICK_TIMER_t;


extern __asm uint32_t nvic_getCurExceptionNum(void);

//Timer resource pool
TIMER_OBJ_t timer[4] = {/*{STATE, Arg2Callback, Timeout value, HW block pointer}*/
						{UNINIT, NULL, 0, NULL, (TIMER_t*)(0x40004000)},
						{UNINIT, NULL, 0, NULL, (TIMER_t*)(0x40008000)},
						{UNINIT, NULL, 0, NULL, (TIMER_t*)(0x40090000)},	
						{UNINIT, NULL, 0, NULL, (TIMER_t*)(0x40094000)}		
							};

uint32_t timer_init(void)
{
	uint32_t i;

	LPC_SC->PCONP |= (TIMER0_PWR_ON | TIMER1_PWR_ON | TIMER2_PWR_ON | TIMER3_PWR_ON);
	
	LPC_SC->PCLKSEL0 |= (TIMER0_CLK_EN | TIMER1_CLK_EN);
	LPC_SC->PCLKSEL1 |= (TIMER2_CLK_EN | TIMER3_CLK_EN);

	
	for(i = 0; i < NO_OF_TIMERS; i++)
	{	
		//Set pre-scale value for all timers such that their resolution is 1usec i.e. PCLK(timer) = CCLK/8 = 9MHz ; 9/PRESCALE = 1MHz
	   timer[i].pHw->PR = PCLK_DIV_BY_FACTOR;
	   //Keep all timers in timer mode
	   timer[i].pHw->CTCR = ((timer[i].pHw->CTCR >> 2) << 2);
	   //Set all timers match condition as: "when matched then generate INT and stop(i.e. monoshot)"
	   timer[i].pHw->MCR = INT_AND_STOP;

	   //Enable interrupt from timer HW	to CM3 core
	   nvic_enableIRQ(TIMER_ID(i));
	   //Set state of all timers as initialized
	   timer[i].state = FREE;
	}

	return 1;
} 

TIMER_OBJ_t*  timer_oneshotAcquire(	uint32_t timeoutValue, 
									void (*timer_expiryHandler)(void *pArg),
									void *pArg)
{
	uint32_t i;
	TIMER_OBJ_t* pRetTimer = NULL;
	
	//Search free timer
	for(i = 0; i < NO_OF_TIMERS; i++)
	{
		 //critical section start
		 if(timer[i].state == FREE)
		 {
		 	pRetTimer = &timer[i];
			pRetTimer->state = ACQUIRED;
			pRetTimer->timer_expiryHandler = timer_expiryHandler;
			pRetTimer->pArg = pArg;
			pRetTimer->pHw->MR[0] = timeoutValue;
			return pRetTimer;
		 }
		 //critical section end
	}
	return pRetTimer;
}

uint32_t timer_start(TIMER_OBJ_t* pTimer)
{
	if(	pTimer->pHw->MR[0] != 0)
	{
		//Reset timer counters: PS and TC
		pTimer->pHw->TCR |= (0x1 << 0x1);
		pTimer->pHw->TCR &= ~(0x1 << 0x1);

		//Turn on the timer
		pTimer->pHw->TCR  |= 0x1;

		pTimer->state = RUNNING;

		return 1;
	}
	return 0;
}

uint32_t timer_stop(TIMER_OBJ_t* pTimer)
{
	if(	pTimer->state == RUNNING)
	{
		//Turn off the timer
		pTimer->pHw->TCR  &= ~(0x1);

		pTimer->state = STOPPED;

		return 1;
	}
	return 0;
}

uint32_t timer_resume(TIMER_OBJ_t* pTimer)
{
	if(	pTimer->state == STOPPED)
	{
		//Turn on the timer
		pTimer->pHw->TCR  |= 0x1;

		pTimer->state = RUNNING;

		return 1;
	}
	return 0;
}

uint32_t timer_release(TIMER_OBJ_t* pTimer)
{
		//Turn off the timer
		pTimer->pHw->TCR  &= ~(0x1);
		pTimer->state = FREE;
		return 1;
}

void timer_isr(void)
{								  
   uint32_t i;
   TIMER_OBJ_t* pTimer;
   _disable_irq();
   
   i = 	nvic_getCurExceptionNum();
   i = (i - TIMER0_EXCEPTION_NUM);
   pTimer = &timer[i];
   pTimer->state = EXPIRED;
  if(pTimer->timer_expiryHandler != NULL) pTimer->timer_expiryHandler(pTimer->pArg);
   
   //Clear INT at timer hw block level
   pTimer->pHw->IR = pTimer->pHw->IR;
   
   //Clear INT at cm3 core level
   nvic_scrubPendingIRQ();

    _enable_irq();
}

TIMER_OBJ_t*  timer_freeRunningStart()
{
	return NULL;
}