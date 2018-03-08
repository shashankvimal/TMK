#include "lpc17xx.h"
#include "type.h"
#include "utils.h"

#define BASE_ADR_WDT				(0x40000000)
#define WATCHDOG_TIMER				(WDT_t*)(BASE_ADR_WDT)
#define WATCHDOG_TIMER_ENABLED() 	(WATCHDOG_TIMER->WDMOD & 0x1)
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
	/*Enable wdt*/
	uint8_t enableWdt;
	/*Select the source of clock for WDT timer block.                                 */
	uint8_t runOnSleepNeeded;
	/*Select the time unit: {WDT_TIMEOUT_UNIT_USEC, WDT_TIMEOUT_UNIT_MSEC}*/
	uint8_t timeUnit;
}WDT_INIT_PARAM_t;

uint32_t 
wdt_init(WDT_INIT_PARAM_t *pParam)
{
	WDT_CLK_SRC_e wdtClkSrc;
	if(wdt_validInitParam(pParam))
	{
		wdtClkSrc =  wdt_determineClkSrc(pParam->timeUnit, pParam->runOnSleepNeeded);
		WATCHDOG_TIMER->WDCLKSEL  = wdtClkSrc | (pParam->timeRebaseNeeded ? (1 << 31) : 0);					
		WATCHDOG_TIMER->WDTC  = wdt_convertTm2Ticks(pParam->timeout, wdtClkSrc);
		WATCHDOG_TIMER->WDMOD = 0x0;
		WATCHDOG_TIMER->WDMOD = (resetOnExpiry ? pParam->mode | (pParam->enableWdt == 1));	
		return 1;
	}
	return 0;

}

uint32_t wdt_feed(void)
{
	if(WATCHDOG_TIMER_ENABLED()) 
	{
		WATCHDOG_TIMER->WDFEED = WDT_FEED_VALUE;
		WATCHDOG_TIMER->WDFEED = TOGGLE(WDT_FEED_VALUE);
		return 1;
	}
	return 0;
}
INLINE static WDT_CLK_SRC_e 
wdt_determineClkSrc(uint32_t timeUnit,
                    uint32_t runOnSleep)
{
	if(runOnSleep == TRUE)
	{
		return WDT_INTERNAL_OSC_CLK;
	}
	if(timeUnit = WDT_TIMEOUT_UNIT_USEC)
	{
		return WDT_PERIPHERAL_CLK;
	}
	if(timeUnit = WDT_TIMEOUT_UNIT_MSEC)
	{
		return WDT_REAL_TIME_CLK;
	}
	return WDT_INVALID_CLK;
}
INLINE static uint32_t 
wdt_validCfgParam(WDT_INIT_PARAM_t *pParam)
{
	return (pParam->clkSrc >= 0 && pParam->clkSrc < 0x3) &&
		   (pParam->timeRebaseNeeded <= 0x1) &&
}

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
	/*Set the scaling factor*/
	/*Read the current count register*/
}
uint32_t wdt_reloadTimeout(uint32_t newTimeoutVal, uint32_t tmUnit)
{

}
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
WDT_CFG_SLEEP_ACT_e 
wdt_determineSleepStatus()
{
	return (((WATCHDOG_TIMER->WDCLKSEL & 0x3) == WDT_INTERNAL_OSC_CLK) ?\
			 WDT_CFG_RUN_ON_SLEEP : WDT_CFG_STOP_ON_SLEEP);
}

void wdt_isr(void)
{

}