//******************************************************************************
//  Filename: lab2-3-2.c
//  Send status of green LED over UART when pressing button S2
//  Author: Kumar Amit Mehta <gmate.amit@gmail.com>
//
//  Steps:
//  - Configure CPU and Watchdog
//  - Configure Ports
//  - Caliberate DCO internal oscillator
//  - Initialize UART module with the speed of 9600 bps
//  - setup button/switch
//  - Use a SMCLK mode 2 to generate timer interrupt
//  - Use the timer ISR to toggle green LED(P1.6)
//  - Send the status of green LED (LED1) over UART when pressing button S2
//******************************************************************************
#include <msp430.h>

// Counter for ~6 seconds delay
unsigned int counter = 90;
unsigned int timer = 0;

void setup_uart(void);
void caliberate_clock(void);
void setup_switch(void);
void setup_delay(void);

int main(void)
{
	// Stop WDT
	WDTCTL = WDTPW + WDTHOLD;

	//Configure CPU clock
	caliberate_clock();

	setup_delay();

	//Green Led, set as output port
	P1DIR |= BIT6;
	setup_uart();

	setup_switch();

	// Enter LPM0 w/ interrupt
	_BIS_SR(LPM0_bits + GIE);
}

void setup_switch(void)
{
	// P1.3 as input(no need to configure, input(default) mode explicitly)
	// Port 1.3 interrupt enable
	P1IE |= BIT3;

	//edge triggered
	P1IES |= BIT3;
	//Port 1 resistor enable
	P1REN |= BIT3;
	//clear any pending interrupts on Port 1.3
	P1IFG &= ~BIT3;
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

void caliberate_clock(void)
{
	// If calibration constant erased
	// Setting DCO to 1MHz frequency
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
	//Maximum value that CCR0 can hold
	CCR0 = 65535;

	//Count from 1 to CCR0, SMCLK, up mode
	TACTL = TASSEL_2 + MC_1;
}

// PORT1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void port_1(void)
{
	//Clear pending interrupts on S2
	P1IFG &= ~BIT3;
	UCA0TXBUF = P1IN & BIT6;
}

//Use interrupt routine, to keep changing the led status
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	// Re-initialize CCR0
	CCR0 += 65535;
	timer++; // Increase timer1 value
	if (timer >= counter) {
		// toggle green LED1
		P1OUT ^= BIT6;
		timer = 0;
	}
}
