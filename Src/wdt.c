/**
* @file: wdt.c
* @brief: This file contains implementation of watchdog driver
* @author:  SHASHANK VIMAL
* @date: 3/24/2018
*/
#include "lpc17xx.h"
#include "type.h"
#include "utils.h"

#define BASE_ADR_WDT				(0x40000000)
#define WATCHDOG_TIMER				((WDT_t*)(BASE_ADR_WDT))
#define WATCHDOG_TIMER_ENABLED() 	(WATCHDOG_TIMER->WDMOD & 0x1)
#define WDT_TIMEOUT_UNIT_USEC       (0)
#define WDT_TIMEOUT_UNIT_MSEC       !(WDT_TIMEOUT_UNIT_USEC)
#define WDT_MIN_COUNT               (0xFF)
#define WDT_FEED_VALUE				(0xAA)

typedef volatile struct wdt
{
	/*RW*/uint32_t WDMOD;			  
	/*RW*/uint32_t WDTC;			 
	/*WO*/uint32_t WDFEED;	    
	/*RO*/const uint32_t WDTV;   
	/*RW*/uint32_t WDCLKSEL;	  
}WDT_t;

typedef enum{WDT_MODE_RESET_WITH_LOG_EN, WDT_MODE_RESET_WITHOUT_LOG_EN,}WDT_MODE_e;
typedef enum{WDT_CLK_SRC_LOCKED, WDT_CLK_SRC_UNLOCKED}WDT_CLK_SRC_STATUS_e;
typedef enum{WDT_CFG_RUN_ON_SLEEP, WDT_CFG_STOP_ON_SLEEP}WDT_CFG_SLEEP_ACT_e;

typedef enum{/*4MHz */                             WDT_INTERNAL_OSC_CLK,
			 /*PCLK_WDT = CCLK/8 = 72MHz/8 = 9MHz*/WDT_PERIPHERAL_CLK, 
			 /*32KHz*/                             WDT_REAL_TIME_CLK, 
			                                       WDT_INVALID_CLK}WDT_CLK_SRC_e;

typedef struct initParam
{	     
	/*Timeout value if user does not provide or set it explicitly.                   */
	uint32_t timeout;
	/*Reset logging callback*/
	void(*LogOnResetCallback)(uint8_t *pMemDump);
	/*{WDT_MODE_RESET_WITHOUT_LOG_EN, WDT_MODE_RESET_WITH_LOG_EN}tells if WDT can raise interrupt.          */
	uint8_t resetOnExpiry;
	/*{CLK_SRC_LOCKED, CLK_SRC_UNLOCKED}tells if clk src change to WDT is possible.*/				 
	uint8_t timeRebaseNeeded;
	/*Select the source of clock for WDT timer block.                                 */
	uint8_t runOnSleepNeeded;
	/*Select the time unit: {WDT_TIMEOUT_UNIT_USEC, WDT_TIMEOUT_UNIT_MSEC}*/
	uint8_t timeUnit;
}WDT_INIT_PARAM_t, WDT_CFG_t;
#if 0
typedef struct cfg
{	     
	/*Reset logging callback*/
	void(*LogOnResetCallback)(uint8_t *pMemDump);
	/*{WDT_MODE_RESET_WITHOUT_LOG_EN, WDT_MODE_RESET_WITH_LOG_EN}tells if WDT can raise interrupt.          */
	uint8_t resetOnExpiry;
	/*{CLK_SRC_LOCKED, CLK_SRC_UNLOCKED}tells if clk src change to WDT is possible.*/				 
	uint8_t timeRebaseNeeded;

	/*Select the source of clock for WDT timer block.                                 */
	uint8_t runOnSleepNeeded;
	/*Select the time unit: {WDT_TIMEOUT_UNIT_USEC, WDT_TIMEOUT_UNIT_MSEC}*/
	uint8_t timeUnit;
}WDT_CFG_t;
#endif
WDT_CFG_t wdgCfg;
static uint32_t lock;
/******************************************************************************
* @function: wdt_lockAcquire
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
__asm uint32_t wdt_lockAcquire(uint32_t* pLock)
{
	MOV R2,#1
	MOV R1,R0
	LDREX R3,[R1]
	CMP R3,R2
	STREXNE R0,R2,[R1]
	DMB
	CMP R0,#0
	MOVEQ R0,#1
	MOVNE R0,#0
}
/******************************************************************************
* @function: wdt_lockRelease
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
__asm void wdt_lockRelease(uint32_t* pLock)
{
	MOV R2,#0
	MOV R1,R0
	STREX R0,R2,[R1]
	DMB
}
/******************************************************************************
* @function: wdt_determineClkSrc
* @description: This is an internal function for determining the clock source 
* @in: 1. Time unit(msec/usec).
* @in: 2. runOnSleep: Whether WDT HW will run even in power down case
* @return: Clock source 
*******************************************************************************/
INLINE static WDT_CLK_SRC_e 
wdt_determineClkSrc(uint32_t timeUnit,
                    uint32_t runOnSleep)
{
	if(runOnSleep == TRUE)
	{
		return WDT_INTERNAL_OSC_CLK;
	}
	if(timeUnit == WDT_TIMEOUT_UNIT_USEC)
	{
		return WDT_PERIPHERAL_CLK;
	}
	if(timeUnit == WDT_TIMEOUT_UNIT_MSEC)
	{
		return WDT_REAL_TIME_CLK;
	}
	return WDT_INVALID_CLK;
}
/******************************************************************************
* @function: wdt_convertTm2Ticks
* @description: initializes the watch dog hardware block and sets config param 
* @in: 1. pointer to initialization parameters.
* @in: 2. HW clock source to watchdog HW
* @return: Watchdog ticks count
*******************************************************************************/
INLINE static uint32_t 
wdt_convertTm2Ticks(uint32_t timeout, 
                    WDT_CLK_SRC_e wdtClkSrc)
{
	uint32_t scalingFactor;
	switch(wdtClkSrc)
	{
		case WDT_INTERNAL_OSC_CLK: 
		    /* WDCLK = IRC/4 = 4MHz/4 => 1 tick = 1 usec*/
		    scalingFactor = 1; break;		
		case WDT_PERIPHERAL_CLK: 
			/**
		    /*PCLK_WDT = CCLK(72MHz)/2 = 36MHz; 
		    /*WDCLK = PCLK_WDT/4 =>  36MHz/4 = 9MHz;
		    /*1 tick = 1/9 usec ~ 0.1usec => 1 usec ~10ticks
		    **/
		    scalingFactor = 9; break;
		case WDT_REAL_TIME_CLK:
		    /* WDCLK = RTC_CLK/4 = 32KHz/4 => 1 tick = 1/8 msec*/
			scalingFactor = 8; break;
		default:
		    return 0;
	}
	return (scalingFactor * timeout);
}
/******************************************************************************
* @function: wdt_validInitParam
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
INLINE static uint32_t 
wdt_validInitParam(WDT_INIT_PARAM_t *pParam)
{
	return (
		   (pParam->timeRebaseNeeded <= 0x1) && 
		   (pParam->resetOnExpiry <= 0x1) &&
		   (pParam->runOnSleepNeeded <= 0x1) && 
		   (pParam->timeUnit <= WDT_TIMEOUT_UNIT_MSEC) &&
		   (WDT_MIN_COUNT <= wdt_convertTm2Ticks(pParam->timeout, 
		                                         wdt_determineClkSrc(pParam->timeUnit,
		                                                             pParam->runOnSleepNeeded)))
		    );
}

/******************************************************************************
* @function: wdt_timeRebaseAllowed
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
INLINE static uint32_t
wdt_timeRebaseAllowed()
{
	return ((WATCHDOG_TIMER->WDCLKSEL & (1 << 31)) >> 31);
}
/******************************************************************************
* @function: wdt_timeRebaseNeeded
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
INLINE static uint32_t
wdt_timeRebaseNeeded(uint32_t tmUnit)
{
	return (tmUnit != wdgCfg.timeUnit && tmUnit <= WDT_TIMEOUT_UNIT_MSEC);
}
/******************************************************************************
* @function: wdt_restart
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
INLINE static uint32_t
wdt_restart(uint32_t newTimeoutVal, uint32_t newTmUnits)
{
	uint32_t wdtTicks, wdtClkSrc;
	if(newTmUnits <= WDT_TIMEOUT_UNIT_MSEC)
	{
		wdtClkSrc = wdt_determineClkSrc(newTmUnits, wdgCfg.runOnSleepNeeded);
		wdtTicks = wdt_convertTm2Ticks(newTimeoutVal, wdtClkSrc);
		                               
	    if(WDT_MIN_COUNT <= wdtTicks)
		{
			/// 1. Disable Watchdog HW unit
			WATCHDOG_TIMER->WDMOD     = 0x0;

			/// 3. Set the clock source and lock it if the user sets so
			WATCHDOG_TIMER->WDCLKSEL  = wdtClkSrc | (wdgCfg.timeRebaseNeeded ? (1 << 31) : 0);					
			/// 4. Load the watchdog timer's counter register
			WATCHDOG_TIMER->WDTC      = wdtTicks;
			/// 5. Apply final configuration to watchdog timer
			WATCHDOG_TIMER->WDMOD     = (((wdgCfg.resetOnExpiry == WDT_MODE_RESET_WITH_LOG_EN) ?
										 0 : (1 << 1)) | 0x1);
			return 1;
	    }
	}
	return 0;
}
/******************************************************************************
* @function: wdt_start
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t wdt_start()
{
	uint32_t retVal;
	if((retVal = wdt_lockAcquire(&lock)) == 1)
	{
		retVal = wdt_restart(wdgCfg.timeout, wdgCfg.timeUnit);
	}
	return retVal;
}
/******************************************************************************
* @function: wdt_stop
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
void wdt_stop()
{
	if(wdt_lockAcquire(&lock))
	{
		uint32_t curCfg = WATCHDOG_TIMER->WDMOD;
		WATCHDOG_TIMER->WDMOD = (curCfg >> 1) << 1;
	}
	wdt_lockRelease(&lock);
}

/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t 
wdt_init(WDT_INIT_PARAM_t *pParam)
{
	if(wdt_validInitParam(pParam))
	{
		if(wdt_lockAcquire(&lock))
		{
			/// 1. Copy the init parameters as current configuration of WDT HW
			wdgCfg.timeout            = pParam->timeout;
			wdgCfg.LogOnResetCallback = pParam->LogOnResetCallback;
			wdgCfg.resetOnExpiry      = pParam->resetOnExpiry;
			wdgCfg.timeRebaseNeeded   = pParam->timeRebaseNeeded;
			wdgCfg.runOnSleepNeeded   = pParam->runOnSleepNeeded;
			wdgCfg.timeUnit           = pParam->timeUnit;
		
			/// 2. Apply final configuration to watchdog timer
			if(pParam->resetOnExpiry == WDT_MODE_RESET_WITH_LOG_EN)
			{
				nvic_enableIRQ(WDT_IRQn);
			}
		
		}
		wdt_lockRelease(&lock);
		/// 3. Return SUCCESS
		return SUCCESS;
	}
	return FAILURE;

}
/******************************************************************************
* @function: wdt_determineSleepStatus
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
WDT_CFG_SLEEP_ACT_e 
wdt_determineSleepStatus()
{
	return (((WATCHDOG_TIMER->WDCLKSEL & 0x3) == WDT_INTERNAL_OSC_CLK) ?\
			 WDT_CFG_RUN_ON_SLEEP : WDT_CFG_STOP_ON_SLEEP);
}
/******************************************************************************
* @function: wdt_reRun
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t wdt_reRun(uint32_t newTimeoutVal, uint32_t newTmUnit)
{
	uint32_t retVal = 0;
	if(wdt_lockAcquire(&lock))
	{
		if(wdt_timeRebaseNeeded(newTmUnit) && wdt_timeRebaseAllowed())
		{
			retVal = wdt_restart(newTimeoutVal, newTmUnit);
		}
	}
    wdt_lockRelease(&lock);	
    return retVal;
}
/******************************************************************************
* @function: wdt_remainingTime4Trigger
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t wdt_remainingTime4Trigger()
{
    uint32_t curTicksElapsed;
	curTicksElapsed = (WATCHDOG_TIMER->WDTC - WATCHDOG_TIMER->WDTV);
	/*Determine clock source*/
	switch(WATCHDOG_TIMER->WDCLKSEL & 0x3)
	{
		case(WDT_INTERNAL_OSC_CLK): return curTicksElapsed;
		case(WDT_PERIPHERAL_CLK): return (curTicksElapsed / 9);
		case(WDT_REAL_TIME_CLK): return (curTicksElapsed / 8);
		default: return 0;
	}
}
/******************************************************************************
* @function: wdt_feed
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t wdt_feed(void)
{
	uint32_t retVal = 0;
	if(wdt_lockAcquire(&lock) && WATCHDOG_TIMER_ENABLED()) 
	{
		WATCHDOG_TIMER->WDFEED = WDT_FEED_VALUE;
		WATCHDOG_TIMER->WDFEED = TOGGLE(WDT_FEED_VALUE);
		retVal = 1;
	}
	wdt_lockRelease(&lock);	
	return retVal;
}

/******************************************************************************
* @function: wdt_isr
* @description: Interrupt service routine for watchdog timer interrupt 
* @in: None.
* @return: None.
*******************************************************************************/
void wdt_isr(void)
{
	nvic_disableIRQ(WDT_IRQn);
	
	if(wdgCfg.LogOnResetCallback != NULL)
		wdgCfg.LogOnResetCallback(NULL);
	
	nvic_enableIRQ(WDT_IRQn);
	__DMB();
	system_softReset();
	while(1);
}