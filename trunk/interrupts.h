#ifndef INTERRUPTS_H
#define INTERRUPTS_H
/*
 *  interrupts.h
 */

/*Interrrupt typedefs*/
typedef void (* interrupt_t) (void);

/*
Structure that holds the function pointers
to each of the HC11's supported interrupt
service routines
*/
struct interrupt_vectors
{
   interrupt_t unused0; /* unused slots in interrupt vector */
   interrupt_t unused1;
   interrupt_t unused2;
   interrupt_t unused3;
   interrupt_t unused4;
   interrupt_t unused5;
   interrupt_t unused6;
   interrupt_t unused7;
   interrupt_t unused8;
   interrupt_t unused9;
   interrupt_t unused10;
 
   interrupt_t sci_handler; /* serial vectors */
   interrupt_t spi_handler;
 
   interrupt_t acc_input_handler;
   interrupt_t acc_overflow_handler;
 
   interrupt_t timer_overflow_handler; /* TCNT Overflow Vector */
   
   interrupt_t output5_handler; /* Timer Output Compare Vectors */
   interrupt_t output4_handler;
   interrupt_t output3_handler;
   interrupt_t output2_handler;
   interrupt_t output1_handler;
 
   interrupt_t capture3_handler; /* Timer Input Compare Vectors */
   interrupt_t capture2_handler;
   interrupt_t capture1_handler;
 
   interrupt_t rtii_handler; /* Real-Time-Interrupt Generator Vector */
 
   interrupt_t irq_handler;
   interrupt_t xirq_handler;
 
   interrupt_t swi_handler; /* Trap Vector */
 
   interrupt_t illegal_handler;
 
   interrupt_t cop_fail_handler; /* COP vectors */
   interrupt_t cop_clock_handler;
 
   interrupt_t reset_handler; /* Reset Vector */
};

typedef struct interrupt_vectors interrupt_vectors_t;

#endif
