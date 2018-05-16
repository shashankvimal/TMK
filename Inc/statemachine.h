#if !defined _STATE_MACHINE_H__
#define  _STATE_MACHINE_H__
#include "type.h"

typedef struct
{
	u32 nextState;
	u32 (*action)(void* pArg);
	void *pArg;
}EVENT_STATE_MATRIX_t;
#endif //END_OF_STATE_MACHINE_H__
