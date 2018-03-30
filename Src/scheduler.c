/******************************************************************************
* @file  scheduler.c
* @brief This file contains interfaces for tasks schedules
* @author  SHASHANK VIMAL 
* @date 3/21/2018
*******************************************************************************/
#include "lpc17xx.h"
#include "type.h"
#include <utils.h>
#include "list.h"

NODE_t* pTmkRdyTaskQ;
NODE_t* pTmkWaitingTaskQ;
TASK_t* pCurrentTask;
/******************************************************************************
* @function list create
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
__asm void tmk_SchedulerInit(TASK_t *pIdleTask)
{
	
}
/******************************************************************************
* @function list create
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
__asm void tmk_schedulerCs2Isr()
{
	
}
/******************************************************************************
* @function list create
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
__asm void tmk_schedulerCs2Task()
{
	
}
/******************************************************************************
* @function list create
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
__asm void tmk_schedule(TASK_t *pdestTask)
{
	
}