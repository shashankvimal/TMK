/**
* @file wdt.c
* @brief This file contains implementation of watchdog driver
* @author  SHASHANK VIMAL
* @date 3/24/2018
*/
#include "lpc17xx.h"
#include "type.h"
#include "utils.h"

#define BASE_ADR_WDT				(0x40000000)
#define WATCHDOG_TIMER				(WDT_t*)(BASE_ADR_WDT)
#define WATCHDOG_TIMER_ENABLED() 	(WATCHDOG_TIMER->WDMOD & 0x1)
#define WDT_TIMEOUT_UNIT_USEC       (0)
#define WDT_TIMEOUT_UNIT_MSEC       !(WDT_TIMEOUT_UNIT_USEC)
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
WDT_CFG_t wdgCfg;


/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
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
/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
INLINE static uint32_t 
wdt_validCfgParam(WDT_INIT_PARAM_t *pParam)
{
	return (pParam->clkSrc >= 0 && pParam->clkSrc < 0x3) &&
		   (pParam->timeRebaseNeeded <= 0x1) &&
}

/******************************************************************************
* @function: wdt_init
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
* @function: wdt_init
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
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
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
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t 
wdt_init(WDT_INIT_PARAM_t *pParam)
{
	WDT_CLK_SRC_e wdtClkSrc;
	if(wdt_validInitParam(pParam))
	{
		/*Disable Watchdog HW unit*/
		WATCHDOG_TIMER->WDMOD     = 0x0;
		/*Determine the clock source*/
		wdtClkSrc                 =  wdt_determineClkSrc(pParam->timeUnit, pParam->runOnSleepNeeded);
		/*Set the clock source and lock it if the user sets so*/
		WATCHDOG_TIMER->WDCLKSEL  = wdtClkSrc | (pParam->timeRebaseNeeded ? (1 << 31) : 0);					
		/*Load the watchdog timer's counter register*/
		WATCHDOG_TIMER->WDTC      = wdt_convertTm2Ticks(pParam->timeout, wdtClkSrc);
		/*Apply final configuration to watchdog timer*/
		WATCHDOG_TIMER->WDMOD     = (resetOnExpiry ? pParam->mode | (pParam->enableWdt == 1));
		
		wdgCfg.LogOnResetCallback = pParam->LogOnResetCallback;
		wdgCfg.resetOnExpiry      = pParam->resetOnExpiry;
		wdgCfg.timeRebaseNeeded   = pParam->timeRebaseNeeded;
		wdgCfg.runOnSleepNeeded   = pParam->runOnSleepNeeded;
		wdgCfg.timeUnit           = pParam->timeUnit;
	}
	return 0;

}
/******************************************************************************
* @function: wdt_init
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
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t wdt_reloadTimeout(uint32_t newTimeoutVal, uint32_t tmUnit)
{
    if(wdt_timeRebaseNeeded(tmUnit))
	   if(wdt_timeRebaseAllowed())
		   wdt_modifyClkSrc();
	   else
		   return 0;
}
/******************************************************************************
* @function: wdt_init
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
	/*Set the scaling factor*/
	/*Read the current count register*/
}
/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
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
/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
void wdt_isr(void)
{

}