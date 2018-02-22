#ifndef __NVIC_H__
#define __NVIC_H__

extern void nvic_enableIRQ(uint32_t);
extern void nvic_disableIRQ(uint32_t);
extern void _enable_irq(void);
extern void _disable_irq(void);
extern void nvic_scrubPendingIRQ(void);

#endif 