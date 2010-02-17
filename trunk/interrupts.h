#ifndef INTERRUPTS_H
#define INTERRUPTS_H
/*
 *  interrupts.h
 */
typedef void (* interrupt_t) (void); 

#define VECTOR_BASE     0xBFC0


#define IVTOI   (*(interrupt_t *)(VECTOR_BASE + 0x1E))

#define TOC5V   (*(interrupt_t *)(VECTOR_BASE + 0x20))
#define IVOC4   (*(interrupt_t *)(VECTOR_BASE + 0x22))
#define TOC3V   (*(interrupt_t *)(VECTOR_BASE + 0x24))
#define TOC2V   (*(interrupt_t *)(VECTOR_BASE + 0x26))
#define TOC1V   (*(interrupt_t *)(VECTOR_BASE + 0x28))
#define IC3V    (*(interrupt_t *)(VECTOR_BASE + 0x2A))
#define IC2V    (*(interrupt_t *)(VECTOR_BASE + 0x2C))
#define IC1V    (*(interrupt_t *)(VECTOR_BASE + 0x2E))
#define RTIV    (*(interrupt_t *)(VECTOR_BASE + 0x30))
//#define     (*(interrupt_t *)(VECTOR_BASE + 0x32))
//#define     (*(interrupt_t *)(VECTOR_BASE + 0x34))
#define IVSWI   (*(interrupt_t *)(VECTOR_BASE + 0x36)) 
#define IVReset (*(interrupt_t *)(VECTOR_BASE + 0x3E))

#endif
