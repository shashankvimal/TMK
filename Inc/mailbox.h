#if !defined __MAILBOX_H__
#define 	__MAILBOX_H__
#include "ringbuffer.h"

typedef enum
{
	TMK_MSG_TYP_CHAINED,
	TMK_MSG_TYP_EVENT,
	TMK_MSG_TYP_POSTCARD, //single message: may contain a data word
}TMK_MBX_MSG_TYP_e;
typedef struct
{
    TMK_MBX_MSG_TYP_e msgType;
	union
	{
		uint32_t data;
		uint8_t  bData[4];
		uint32_t *pList;
	}body;
}MBX_MSG_CONTAINER_t;
typedef struct mailbox
{
    uint8_t mbxId;
	uint8_t mbxProcessedMsgCnt;
	uint8_t mbxDeferredMsgCnt;
	uint8_t mbxDeferLimit;
	RING_BUFFER_t mbxMsgDeferQ;
	uint8_t pad1;
	uint8_t pad2;
	uint8_t mbxLock;
	uint8_t mbxMsg2Ignore;
	uint32_t (*mbxTlvParser)(uint8_t* pMsg);
	MBX_MSG_CONTAINER_t msg;
}MAILBOX_t;
#endif //END_OF_MAILBOX_H__
