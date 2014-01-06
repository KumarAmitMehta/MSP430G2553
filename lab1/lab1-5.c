//******************************************************************************
//  Filename: lab1-5.c
//  Display, blink RED LED with delay, using SMCLK
//
//  Steps:
//  - Configure CPU and Watchdog
//  - Configure Ports 
//  - Configure timer
//
//  - Display, blink RED LED with delay, using SMCLK
//******************************************************************************
#include <msp430.h>
unsigned int counter = 80; //Counter for 5 seconds delay

// explanation on the magic number 80 above!!!
// Freq = 1MHZ, therefore T = 1/freq = 1us
// To count 1 second, need to multiply 1us by 1000000(10 ^6), but the 
// counter(CCR0) is just 16 bits wide, so the maximum that it can count is 2^16
// therfore dividing 10^6 with 2^16(1000000/65535) yields ~15 which means if the
// CCR0 counter has to overflow 15 times to generate a delay
// of 1 second, and that's why, I need CCR0 to overflow ~80(rounding up to 80)
// times to give me a delay of 5 seconds.

unsigned int timer = 0;
void setup_delay();
void caliberate_clock(void);

void caliberate_clock(void)
{
    // If calibration constant erased
    if (CALBC1_1MHZ==0xFF)       
    {    
        // do not load, trap CPU
        while(1);                               
    }
}

void setup_delay()
{    
    // Select lowest DCOx and MODx settings
    DCOCTL = 0;                               
    // Set DCO
    BCSCTL1 = CALBC1_1MHZ;                    
    DCOCTL = CALDCO_1MHZ;
    
    // Configure Timer_A to generate interrupt, when overflow occurs
    // CCR0 interrupt enabled
    CCTL0 = CCIE;                             
    CCR0 = 65535;
    
    // Count from 1 to CCR0, SMCLK, up mode 
    TACTL = TASSEL_2 + MC_1;
}

int main(void)
{
    // Stop Watchdog timer
    WDTCTL = WDTPW + WDTHOLD;
    // Configure CPU clock
    caliberate_clock();
    setup_delay();
    // RED Led, set as output port
    P1DIR |= BIT0;
    // Enter LPM0 w/ interrupt
    _BIS_SR(LPM0_bits + GIE);                   
}

// Use interrupt routine, to keep changing the led status
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    // If Timer_A, mode 2 selected then the value in CCR0 will not have any effect
    // Re initialize CCR0 for the next interrupt 
    CCR0 += 65535;
    
    // Increase timer1 value
    timer++; 
    
    if (timer >= counter) {
        // toggle Red LED1
        P1OUT ^= BIT0;                               
        timer = 0;
    }
}
