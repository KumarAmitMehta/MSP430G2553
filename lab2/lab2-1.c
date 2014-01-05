//******************************************************************************
//  Filename: lab2-1.c
//  Send data over UART to PC
//  Author: Kumar Amit Mehta <gmate.amit@gmail.com>
//
//  Steps:
//
//  - Configure CPU and Watchdog
//  - Configure Ports
//  - Caliberate DCO internal oscillator
//  - Initialize UART module with the speed of 9600 bps
//  - start sending data over UART
//******************************************************************************

#include <msp430.h>
volatile unsigned int i = 0;

void caliberate_clock(void);
void setup_uart(void);

int main(void)
{
	// Stop WDT
	WDTCTL = WDTPW + WDTHOLD;

	//Configure CPU clock
	caliberate_clock();

	setup_uart();

	//start sending over UART
	for (;;) {
		for (i = 0; i < 5000; i++);
		//Transmission is initiated by writing data to UCAxTXBUF
		UCA0TXBUF = 0x0A;
	}
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
