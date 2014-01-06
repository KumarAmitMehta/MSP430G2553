//******************************************************************************
// Filename: lab3-1.c
// Measure CPU temperature with ADC and send it to PC
// Author: Kumar Amit Mehta <gmate.amit@gmail.com> 
//
// Steps: 
// - Configure CPU and Watchdog
// - Configure DCO to work at 1 MHz
// - Configure UART Ports
// - Configure ADC control register
// - Start sampling and conversion using ECN and ADC10OSC
// - Conversion result, stored in ADC10MEM
// - Use formula as ADC10MEM*423/1024 - 278
// - Use UCA0TXBUF to send temperature over UART to PC once per second
//******************************************************************************
#include <msp430.h>

void setup_uart(void);
void caliberate_clock(void);
void setup_adc(void);

//Counter for 1 second delay
unsigned int counter = 15; 
unsigned int timer = 0;
unsigned int i;

int main(void)
{
	// Stop WDT
	WDTCTL = WDTPW + WDTHOLD;                 
	caliberate_clock();

	// Select lowest DCOx and MODx settings
	DCOCTL = 0;
	// Set DCO
	BCSCTL1 = CALBC1_1MHZ;                    
	DCOCTL = CALDCO_1MHZ;
	// Configure Timer
	// CCR0 interrupt enabled
	CCTL0 = CCIE;                             
	CCR0 = 30000;
	// CCR0 = 65535;

	// SMCLK, continuous mode
	TACTL = TASSEL_2 + MC_1;                  
	setup_uart();

	setup_adc();
	
	// Enter LPM0 w/ interrupt
	_BIS_SR(LPM0_bits + GIE);                 
}

void setup_uart(void)
{
	// UART setup section
	// Refer data sheet, pg 43
	P1SEL = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2; // SMCLK

	// 1MHz 9600 (N=BRCLK/baudrate)
	UCA0BR0 = 0x6D;  //refer user guide
	UCA0BR1 = 0; // 1MHz 9600
	UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
}

void caliberate_clock(void)
{
	// If calibration constant erased
	if (CALBC1_1MHZ==0xFF)    
	{
		while(1); // do not load, trap CPU
	}
}

void setup_adc(void)
{
	// ADC10 control register 1 - Select the in-built temperature sensor
	// ADC10 control register 0 - Select sample and hold time, turn on ADC10
	// Select reference (Vr+ and Vr-
	ADC10CTL1 = INCH_10 + ADC10DIV_3;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	// Add Offset to CCR0 
	CCR0 += 30000;

	// Increase timer1 value 
	timer++; 
	if (timer >= counter) {
		// start the conversion using ADC10SC as source for SHI 

		ADC10CTL0 |= ENC + ADC10SC; 

		var = ADC10MEM*423/1024 -278;

		// Send data over UART
		UCA0TXBUF = var;

		for (i = 0; i < 50000; i++);

		timer = 0;
	}
}
