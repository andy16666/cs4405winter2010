#ifndef INTERRUPTS_H
#define INTERRUPTS_H


/*
 *  interrupts.h
 */

//extern volatile struct interrupt_vectors __attribute__((section("vectors"))) IV; 

void UnhandledInterrupt (void) __attribute__((interrupt));  
void Reset (void) __attribute__((interrupt)); 

/*Interrrupt typedefs*/
typedef void (* interrupt_t) (void);

/*
Structure that holds the function pointers
to each of the HC11's supported interrupt
service routines
*/
struct interrupt_vectors
{
   interrupt_t unused0;  /* 0xC0 unused slots in interrupt vector */
   interrupt_t unused1;  /* 0xC2 */ 
   interrupt_t unused2;  /* 0xC4 */ 
   interrupt_t unused3;  /* 0xC6 */ 
   interrupt_t unused4;  /* 0xC8 */ 
   interrupt_t unused5;  /* 0xCA */ 
   interrupt_t unused6;  /* 0xCC */ 
   interrupt_t unused7;  /* 0xCE */ 
   interrupt_t unused8;  /* 0xD0 */ 
   interrupt_t unused9;  /* 0xD2 */ 
   interrupt_t unused10; /* 0xD0 */ 
 
   interrupt_t sci_handler; /* serial vectors */
   interrupt_t spi_handler;
 
   interrupt_t acc_input_handler;
   interrupt_t acc_overflow_handler;
 
   interrupt_t TOI; /* Timer Overflow Interrupt Handler. */
   
   interrupt_t OC5; /* Timer Output Compare Vectors */
   interrupt_t OC4;
   interrupt_t OC3;
   interrupt_t OC2;
   interrupt_t OC1;
 
   interrupt_t IC3; /* Timer Input Compare Vectors */
   interrupt_t IC2;
   interrupt_t IC1;
 
   interrupt_t RTII; /* Real-Time-Interrupt Generator Vector */
 
   interrupt_t IRQ;
   interrupt_t XIRQ;
 
   interrupt_t SWI; /* Trap Vector */
 
   interrupt_t illegal_handler;
 
   interrupt_t cop_fail_handler; /* COP vectors */
   interrupt_t cop_clock_handler; 
 
   interrupt_t Reset; /* Reset Vector */
};



#endif
