/* The interrupt vector. */ 
extern volatile struct interrupt_vectors __attribute__((section("vectors"))) IV = {
   unused0:                UnhandledInterrupt,
   unused1:                UnhandledInterrupt,
   unused2:                UnhandledInterrupt,
   unused3:                UnhandledInterrupt,
   unused4:                UnhandledInterrupt,
   unused5:                UnhandledInterrupt,
   unused6:                UnhandledInterrupt,
   unused7:                UnhandledInterrupt,
   unused8:                UnhandledInterrupt,
   unused9:                UnhandledInterrupt, 
   unused10:               UnhandledInterrupt, /* unused interrupt handlers */
   sci_handler:            UnhandledInterrupt, /* sci - unused */
   spi_handler:            UnhandledInterrupt, /* spi - unused */
   acc_overflow_handler:   UnhandledInterrupt, /* acc overflow - right shaft encoder - calib */
   acc_input_handler:      UnhandledInterrupt, /* acc input - right shaft encoder - turning */
   timer_overflow_handler: UnhandledInterrupt, /* timer overflow handler */
   OC5:                    UnhandledInterrupt, /* out compare 5 - sound */
   OC4:                    UnhandledInterrupt, /* Output Compare 4 (OC4) Preemption */
   OC3:                    UnhandledInterrupt, /* out compare 3 - unused */
   OC2:                    UnhandledInterrupt, /* out compare 2 - unused */
   OC1:                    UnhandledInterrupt, /* out compare 1 - unused */
   capture3_handler:       UnhandledInterrupt, /* in capt 3 -left shaft encoder -calib & turning*/
   capture2_handler:       UnhandledInterrupt, /* in capt 2 - unused */
   capture1_handler:       UnhandledInterrupt, /* in capt 1 - unused */
   irq_handler:            UnhandledInterrupt, /* IRQ - unused */
   xirq_handler:           UnhandledInterrupt, /* XIRQ - unused */
   SWI:                    UnhandledInterrupt, /* SWI: Trap function used for voluntary context switching. */ 
   illegal_handler:        UnhandledInterrupt, /* illegal -unused */
   cop_fail_handler:       UnhandledInterrupt, /* unused */
   cop_clock_handler:      UnhandledInterrupt, /* unused */
   rtii_handler:           UnhandledInterrupt, /*  */
   reset_handler:          UnhandledInterrupt  /* reset vector - go to premain */
};

void UnhandledInterrupt(void) {	
	/* Do Nothing. */ 
}
