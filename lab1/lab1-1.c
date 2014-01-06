//******************************************************************************
// Filename: lab1-1.c
// Blink Red LED
//
// Steps:
// - Disable Watchdog
// - Select the output port corresponding to the Red LED on the board
// - blink the LED
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
	P1DIR |= BIT0;

	//Toggle LED in an infinite loop
	for (;;)
	{
		//Toggle P1.0 using exclusive-OR operation
		P1OUT ^= 0x01;
		//Delay between LED toggles.
		for(i=0; i< 20000; i++);
	}
}
