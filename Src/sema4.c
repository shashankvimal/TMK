/**
* @file: sema4.c
* @brief: this file is used to sharing single resource between task.
* @author:  SHASHANK VIMAL
* @date: 3/18/2018
*/
#include "lpc17xx.h"
#include "type.h"
#include <utils.h>
#include "list.h"

typedef  struct sema4
{
	LIST_t tlist;
	uint32_t sema4Id;
	uint32_t ownerId;
	uint32_t state;
	uint32_t mode;
	uint32_t contentionCount;
	uint32_t singleAccessEn;
	uint32_t contentionCountLimit;
}sema4_t;
/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t tmk_sema4Acquire(sema4_t *pSema4)
{
	
}
/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t tmk_sema4Release(sema4_t *pSema4)
{
	
}
/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t tmk_sema4Convert2Mutex(sema4_t *pSema4)
{
	
}
/******************************************************************************
* @function: wdt_init
* @description: initializes the watch dog hardware block and sets config param 
* @in: pointer to initialization parameters.
* @return Status
*******************************************************************************/
uint32_t tmk_sema4Attempt2Acquire(sema4_t *pSema4)
{
	
}