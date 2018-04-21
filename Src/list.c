/******************************************************************************
* @file  list.c
* @brief This file contains interfaces for managing circular linked
* list
* @author  SHASHANK VIMAL 
* @date 3/21/2018
*******************************************************************************/
#include "lpc17xx.h"
#include "type.h"
#include <utils.h>
#include "list.h"


/******************************************************************************
* @function list create
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
uint32_t list_create(NODE_t *pHead_p)
{
	uint32_t retVal = FAILURE;
    /// -1. Check the head pointer is valid	
	if(pHead_p != NULL)
	{
		pHead_p->pNext = pHead_p;
		pHead_p->pPrev = pHead_p;
		retVal = SUCCESS;
	}
	return retVal;
}
/******************************************************************************
* @function list_insertPrevious
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
uint32_t list_insertPrevious(NODE_t *pSuccessor_p, NODE_t *pNode2Insert_p)
{
	if(pPrev != NULL && pNode2Insert_p != NULL)
	{
		pSuccessor_p->pPrev->pNext = pNode2Insert_p;
		pNode2Insert_p->pPrev = pSuccessor_p->pPrev;
		pNode2Insert_p->pNext = pSuccessor_p;
		pSuccessor_p->pPrev = pNode2Insert_p;
		
		return SUCCESS;
	}
	return FAILURE;
}
/******************************************************************************
* @function list_insertNext
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
uint32_t list_insertNext(NODE_t *pPredecessor, NODE_t *pNode2Insert)
{
	if(pPrev != NULL && pNode2Insert != NULL)
	{
		pNode2Insert->pNext = pPredecessor->pNext;
		pNode2Insert->pPrev = pPredecessor;
		pNode2Insert->pNext->pPrev = pNode2Insert;
        pPredecessor->pNext = pNode2Insert;
		
		return SUCCESS;
	}
	return FAILURE;
}
/******************************************************************************
* @function list_removePrevious
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
uint32_t list_removePrevious(NODE_t **pNode_p, NODE_t *pSuccessor_p)
{
	if(pNode_p != NULL && pSuccessor_p != NULL)
	{
		*pNode_p = pSuccessor_p->pPrev;
		pSuccessor_p->pPrev = pSuccessor_p->pPrev->pPrev;
		pSuccessor_p->pPrev->pNext = pSuccessor_p;
		
		return SUCCESS;
	}
	return FAILURE;
}
/******************************************************************************
* @function list_removeNext
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
uint32_t list_removeNext(NODE_t **pNode_p, NODE_t *pPredecessor_p)
{
	if(pPrev != NULL && pNode2Insert != NULL)
	{
		*pNode_p = pPredecessor_p->pNext;
		pPredecessor_p->pNext = pNode_p->pNext;
		pNode_p->pPrev = pPredecessor_p;
		
		return SUCCESS;
	}
	return FAILURE;
}
/******************************************************************************
* @function list_countNode
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
uint32_t list_countNode(NODE_t *pHead_p)
{
	uint32_t count  = 0;
	
	if(pHead_p != NULL)
	{
		for(pTempNode = pHead_p, count = 1;
		    pTempNode->pNext != pHead_p;
			pTempNode = pTempNode->pNext, count++);
		
	}

	return count;
}
/******************************************************************************
* @function list_visit
* @inout head node.
* @see Test()
* @return Status
*******************************************************************************/
uint32_t list_visit(NODE_t *pHead_p,
                    void (*visitorProcedure_p)(void *pArg_p, NODE_t *pNode_p),
					uint32_t *pNumVisitedNode_p)
{
	uint32_t retVal = SUCCESS;
	pNode_p = pHead_p;
	*pNumVisitedNode_p = 0;
	
	if(pHead_p == NULL || pNumVisitedNode_p == NULL) return FAILURE;
	do
	{
		
		retVal = visitorProcedure_p(pArg_p, pNode_p);
		
		*pNumVisitedNode_p++;
		
		pNode_p = pNode_p->pNext;
	}while(pNode_p != pHead_p && retVal == SUCCESS);
	
	return retVal;
	
}