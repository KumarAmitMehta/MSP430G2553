//******************************************************************************
// Filename: lab1-4.c
// Blink both Red and Green LED alternatively
//
// Steps: 
// - Disable Watchdog
// - Select the output port corresponding to the Red and Green LED on the board
// - blink the LEDs alternatively
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
    
    //Toggle LEDs in an infinite loop
    for (;;)
    {
        P1OUT &= ~BIT6;
        P1OUT |= BIT0;
        for(i=0; i< 20000; i++);
        P1OUT &= ~BIT0;
        P1OUT |= BIT6;
        for(i=0; i< 20000; i++);
    }
}   
