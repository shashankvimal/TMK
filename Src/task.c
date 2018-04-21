/**
* @file: task.c
* @brief: this file is used to provide what work will be going to
* happen next.
* @author:  SHASHANK VIMAL
* @date: 3/18/2018
*/
#include "lpc17xx.h"
#include "type.h"
#include <utils.h>
#include "list.h"

typedef  struct task
{
	uint32_t tUpperSentinel;
	NODE_t tlist;
	uint32_t(key) tld;
	uint32_t tPriority;
	TMK_TASK_STATE_e tState;
	TMK_TASK_TYPE_e tType;
	MAILBOX_t tMailbox;
	EVENT_STATE_MATRIX[][] tAppStateMachine;
	TMK_CORE_CONTEXT_t tContext;
	uint32_t (*tAppCurState)();
	void (*tEntryPoint)();
	void (*tOnReset)();
	uint32_t tLowerSentinel;
	
}TASK_t;