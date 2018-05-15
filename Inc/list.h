#ifndef __LIST_H__
#define __LIST_H__

#if defined(_LIST_IMPL_)
#define extern 
#endif
typedef struct node
{
	struct node *pNext;
	struct node *pPrev;
}NODE_t;
extern uint32_t list_create(NODE_t *pHead_p);
extern uint32_t list_insertPrevious(NODE_t *pSuccessor_p, 
									NODE_t *pNode2Insert_p);
extern uint32_t list_insertNext(NODE_t *pPredecessor,
                                NODE_t *pNode2Insert);
extern uint32_t list_removePrevious(NODE_t **pNode_p,
                                    NODE_t *pSuccessor_p);
extern uint32_t list_removeNext(NODE_t **pNode_p,
								NODE_t *pPredecessor_p);
extern uint32_t list_countNode(NODE_t *pHead_p);
extern uint32_t list_visit(NODE_t *pHead_p,
                    uint32_t (*visitorProcedure_p)(void *pArg_p, NODE_t *pNode_p),
					void *pArg_p,
					uint32_t *pNumVisitedNode_p);
#endif /*END_OF_LIST_H_*/
