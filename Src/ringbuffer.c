
#include <ringbuffer.h>

i32 ringBufferCreate(RING_BUFFER_t *pRb, u8 *pBuffer, u32 size)
{
	if(!pRb || !pRb->pBuff) return 0;
	pRb->pRbRead = pRb->pRbWrite = 0;
	pRb->pBuff = pBuffer;
	return 1;	
}

u32 ringBufferEmpty(RING_BUFFER_t *pRb)
{	u32 nBytesInRb =  (pRb->pRbWrite - pRb->pRbRead) & 	(pRb->size - 1);
	return(nBytesInRb == 0);
}

u32 ringBufferFull(RING_BUFFER_t *pRb)
{
	u32 nBytesInRb =  (pRb->pRbWrite - pRb->pRbRead) & 	(pRb->size - 1);
	return(nBytesInRb == pRb->size);
}

i32 ringBufferWrite(RING_BUFFER_t *pRb, u8 data)
{								
	if(!pRb || !pRb->pBuff || ringBufferFull(pRb)) return 0;
	
	pRb->pBuff[pRb->pRbWrite]  = data;
	pRb->pRbWrite = ((pRb->pRbWrite + 1) & (pRb->size - 1));
	return 1;
}
		
i32 ringBufferRead(RING_BUFFER_t *pRb, u8 *pData)
{
	if(!pRb || !pRb->pBuff || ringBufferEmpty(pRb)) return 0;
	
	*pData = pRb->pBuff[pRb->pRbRead];
	pRb->pRbRead = ((pRb->pRbWrite + 1) & (pRb->size - 1));
	return 1;
}
