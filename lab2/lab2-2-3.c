//******************************************************************************
//  Filename: lab2-2-3.c
//  Send the delay value X as a number over UART to PC in every X seconds
//  Author: Kumar Amit Mehta <gmate.amit@gmail.com>
// 
//  Steps:
//  - Configure CPU and Watchdog
//  - Configure SMCLK up mode
//  - Initialize UART module with the speed of 9600 bps
//  - Configure TIMER_A interrupt
//  - Use TIMER_A ISR to send the delay value 'X' over UART
//******************************************************************************
#include <msp430.h>

// last digit of my matriculation number is 3
unsigned int counter = 60; //Counter for 3 seconds delay
unsigned int timer = 0;

void setup_delay();
void caliberate_clock(void);
void setup_uart(void);

int main(void)
{
	// Stop Watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	// Configure CPU clock
	caliberate_clock();
	
	// setup uart
	setup_uart();

	setup_delay();

	// Enter LPM0 w/ interrupt
	_BIS_SR(LPM0_bits + GIE);                   
}

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

// UART setup section
void setup_uart(void)
{
	// As per data sheet, for UCA0TXD from USCI 
	// P1.1 = RXD, P1.2=TXD
	P1SEL = BIT1 + BIT2 ; 
	// P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 + BIT2 ;
	// USCI A0 Control Register 1
	// clock source SMCLK
	UCA0CTL1 |= UCSSEL_2; 
	// 1MHz 9600 (N=BRCLK/baudrate)
	// USCI_A0 Baud rate control register 0
	UCA0BR0 = 0x6D;
	// 1MHz 9600
	// USCI_A0 Baud rate control register 1
	UCA0BR1 = 0;
	// Modulation UCBRSx = 1
	UCA0MCTL = UCBRS0;
	// **Initialize USCI state machine** 
	UCA0CTL1 &= ~UCSWRST;
}

// Use interrupt routine, to keep changing the led status
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	// Re initialize CCR0 for the next interrupt 
	CCR0 += 65535;

	// Increase timer1 value
	timer++; 

	if (timer >= counter) {
		// send '3' (last digit of my matriculation number over UART) 
		UCA0TXBUF = BIT1|BIT0;
		timer = 0;
	}
}
