/* Part 1:  68HC11 Definitions. */




/* Timing */ 
#define M6811_TCNT_HIGH	0x0E	/* Timer Counter - Read in one instruction. */ 
#define M6811_TCNT_LOW	0x0F
#define M6811_TIC1_HIGH	0x10	/* Input Capture Registers */ 
#define M6811_TIC1_LOW	0x11
#define M6811_TIC2_HIGH	0x12
#define M6811_TIC2_LOW	0x13
#define M6811_TIC3_HIGH	0x14
#define M6811_TIC3_LOW	0x15



#define M6811_TCTL1	0x20
#define M6811_TCTL2	0x21    /* 0:0:EDG1B:EDG1A:EDG2B:EDG2A:EDG3B:EDG3A */
#define M6811_TMSK1	0x22    /* OC1I:OC2I:OC3I:OC4I:OC5I:IC1I:IC2I:IC3I */
#define M6811_TFLG1	0x23    /* OC1F:OC2F:OC3F:OC4F:OC5F:IC1F:IC2F:IC3F */ 
#define M6811_TMSK2	0x24    /* TOI:RTII:PAOVI:PAII:0:0:PR1:PR0 */
#define M6811_TFLG2	0x25    /* TOF:RTIF:PAOVF:PAIF:0:0:0:0 */ 
#define M6811_PACTL	0x26    /* DDRA7:PAEN:PAMOD:PEDGE:0:0:RTR1:RTR0 */ 
/*
TOI: Enable interrupt on overflow. 
TOF: Overflow has occurred.  	
*/

#define M6811_OPTION	0x39    /* ADPU:CSEL:IRQE;DLY:CME:0:CR1:CR0 */ 



#define M6811_BAUD	0x2B	/* SCI Baud register */
#define M6811_SCCR1	0x2C	/* SCI Control register 1 */
#define M6811_SCCR2	0x2D	/* SCI Control register 2 */
#define M6811_SCSR	0x2E	/* SCI Status register */
#define M6811_SCDR	0x2F	/* SCI Data (Read => RDR, Write => TDR) */

/* Flags of the SCCR2 register.  */
#define M6811_TE	0x08	/* Transmit Enable */
#define M6811_RE	0x04	/* Receive Enable */

/* Flags of the SCSR register.  */
#define M6811_TDRE	0x80	/* Transmit Data Register Empty */

/* Flags of the BAUD register.  */
#define M6811_SCP1	0x20	/* SCI Baud rate prescaler select */
#define M6811_SCP0	0x10
#define M6811_SCR2	0x04	/* SCI Baud rate select */
#define M6811_SCR1	0x02
#define M6811_SCR0	0x01

#define M6811_BAUD_DIV_1	(0)
#define M6811_BAUD_DIV_3	(M6811_SCP0)
#define M6811_BAUD_DIV_4	(M6811_SCP1)
#define M6811_BAUD_DIV_13	(M6811_SCP1|M6811_SCP0)

#define M6811_DEF_BAUD M6811_BAUD_DIV_4 /* 1200 baud */

/* The I/O registers are represented by a volatile array.
   Address is fixed at link time.  For this particular example,
   the _io_ports address is defined in the `memory.x' file.  */
extern volatile unsigned short __attribute__((section("ports"))) Ports[];

