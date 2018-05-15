#if !defined(RING_BUFFER_H__)
#define RING_BUFFER_H__
#include <type.h>

#define RESET_RING_BUFFER(pRb)\
do{\
	ringBufferCreate((pRb), (pRb)->pBuff, (pRb)->size);\
}while(0)
											
typedef struct
{
	u32 size;
	u16 pRbRead;
	u16 pRbWrite;
	u8  *pBuff;
}RING_BUFFER_t;

extern i32 ringBufferCreate(RING_BUFFER_t *pRb, u8 *pBuffer, u32 size);
extern i32 ringBufferWrite(RING_BUFFER_t *pRb, u8 data);
extern i32 ringBufferRead(RING_BUFFER_t *pRb, u8 *pData);
extern u32 ringBufferEmpty(RING_BUFFER_t *pRb);
extern u32 ringBufferFull(RING_BUFFER_t *pRb);

#endif //END_OF_RING_BUFFER_H__
