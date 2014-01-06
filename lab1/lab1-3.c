//******************************************************************************
// Filename: lab1-3.c
// Blink both Red and Green LED simulataneously
//
// Steps: 
// - Disable Watchdog
// - Select the output port corresponding to the Red and Green LED on the board
// - blink the LEDs together
//******************************************************************************
#include  <msp430g2553.h>
//Used as delay counter
unsigned int i = 0;

void main(void)
{
	//Stop watchdog timer.
	WDTCTL = WDTPW + WDTHOLD;

	//All ports by default(upon POR, PUC) are in input mode
	//P1DIR is a register that configures the direction (DIR)
	//RED LED is connected to port 1.1
	//Green LED is connected to port 1.6
	P1DIR |= (BIT0 + BIT6);

	//Toggle LED in an infinite loop	
	for (;;)
	{
		//Toggle only P1.1 and P1.6 using exclusive-OR operation
		P1OUT ^= (BIT0 + BIT6);
		//Delay between LED toggles.                           
		for(i=0; i< 20000; i++);
	}
}
