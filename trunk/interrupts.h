#ifndef INTERRUPTS_H
#define INTERRUPTS_H
/*
 *  interrupts.h
 */
typedef void (* interrupt_t) (void); 

#define VECTOR_BASE     0xFFC0

#define TOC5V   (*(interrupt_t *)(VECTOR_BASE + 0x20)) /* TOC 5 */
#define IVOC4   (*(interrupt_t *)(VECTOR_BASE + 0x22)) /* TOC 4 */
#define TOC3V   (*(interrupt_t *)(VECTOR_BASE + 0x24)) /* TOC 3 */
#define TOC2V   (*(interrupt_t *)(VECTOR_BASE + 0x26)) /* TOC 2 */
#define TOC1V   (*(interrupt_t *)(VECTOR_BASE + 0x28)) /* TOC 1 */
#define IC3V    (*(interrupt_t *)(VECTOR_BASE + 0x2A)) /* IC3 */
#define IC2V    (*(interrupt_t *)(VECTOR_BASE + 0x2C)) /* IC2 */
#define IC1V    (*(interrupt_t *)(VECTOR_BASE + 0x2E)) /* IC1 */
#define RTIV    (*(interrupt_t *)(VECTOR_BASE + 0x30)) /* RTI */
#define IVSWI   (*(interrupt_t *)(VECTOR_BASE + 0x36)) /* SWI */
#define IVReset (*(interrupt_t *)(VECTOR_BASE + 0x3E))

#endif
