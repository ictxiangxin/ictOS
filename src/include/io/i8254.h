/*================================================================================*/
/*                              ictOS i8254 I/O Ports                             */
/*                                                                        by: ict */
/*================================================================================*/

#define I8254_COUNTER0      0x40	/* counter0 port number */
#define I8254_COUNTER1      0x41	/* counter1 port number */
#define I8254_COUNTER2      0x42	/* counter2 port number */
#define I8254_MCR           0x43	/* Mode Control Register */

#define I8254_BINARY        0x0	/* counter use binary number */
#define I8254_DECIMAL       0x1	/* counter use decimal number */

#define I8254_ITC           0X0	/* interrupt on terminal count */
#define I8254_POS           0X2	/* programmable one-shot */
#define I8254_RG            0x4	/* rate generator */
#define I8254_SWRG          0x6	/* square wave rate generator */
#define I8254_STS           0X8	/* software triggered strobe */
#define I8254_HTS           0xa	/* hardware triggered strobe */

#define I8254_LATCH         0x0	/* latch */
#define I8254_HIGH          0x10	/* read/write high byte */
#define I8254_LOW           0x20	/* read/wrtte low byte */
#define I8254_LOWHIGH       0x30	/* read/write low byte, then read/write high byte */

#define I8254_FREQUENCY     0x1234dc	/* i8254 work frequency */
