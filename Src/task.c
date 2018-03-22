/**
* @file task.c
* @brief this file is used to provide what work will be going to
* happen next.
* @author  SHASHANK VIMAL
* @date 3/18/2018
*/

ypedef  struct task
{
	/*RW*/uint32_t tUpperSentinel;
	/*RW*/LIST_t tlist;
	/*WO*/uint32_t(key) tld;
	/*RO*/uint32_t tPriority;
	/*RW*/TMK_TASK_STATE_e tState;
	/*RW*/TMK_TASK_TYPE_e tType;
	/*RW*/MAILBOX_t tMailbox;
	/*RW*/EVENT_STATE_MATRIX[][] tAppStateMachine;
	/*RW*/TMK_CORE_CONTEXT_t tContext;
	/*RW*/uint32_t (*tAppCurState)();
	/*RW*/void (*tEntryPoint)();
	/*RW*/void (*tOnReset)();
	/*RW*/uint32_t tLowerSentinel;
	
}task_t;