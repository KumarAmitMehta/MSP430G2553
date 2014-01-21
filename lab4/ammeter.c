//******************************************************************************
// Filename: ammeter.c
// Read analog voltage from the Voltage-divider circuit and convert it to
// corresponding digital value and display that value over UART to Realterm
// terminal running on windows PC.
//
// Steps: 
// - Stop Watchdog
// - Configure DCO to work at 1 MHz
// - Configure UART Ports
// - Configure ADC control register
// - Start sampling and conversion using ECN and ADC10OSC
// - Conversion result, stored in ADC10MEM
// - Use formula as ADC10MEM * 0.003412 to calculate Vout
// - Calculate current by using Ohm's law: Iload = Vout/Ishunt
// - Use UCA0TXBUF to send current measured over UART to PC once per second
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sr 	Vin	Rshunt	Vload	Rload	ADC10MEM Voltage 	Iload(mA) UART
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 1	3.5	10.1	3.15	100	96	 0.0328032	3.28032	  32
// 2	3.5	10.1	3.15	120	80	 0.027336	2.7336	  27
// 3	3.5	10.1	3.15	150	65	 0.0222105	2.22105	  22
// 4	3.5	10.1	3.15	180	54	 0.0184518	1.84518	  18
// 5	3.5	10.1	3.15	220	42	 0.0143514	1.43514	  14
// 6	3.5	10.1	3.15	620	15	 0.0051255	0.51255	  5
// 7	3.5	10.1	3.15	680	14	 0.0047838	0.47838	  4
// 8	3.5	10.1	3.15	692	10	 0.003417	0.3417	  3
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//
//*****************************************************************************

#include <msp430.h>

void setup_uart(void);
void caliberate_clock(void);
void setup_adc(void);

//Counter for 1 second delay
unsigned int counter = 15;
unsigned int timer = 0;
unsigned int i; 
float temp;
float vout;

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

	// SMCLK, continuous mode
	TACTL = TASSEL_2 + MC_1;

	setup_uart();

	//ADC Input pin P1.5
	//Refer data sheet, page 6
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
	UCA0BR0 = 0x6D; //refer user guide
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
	P1SEL |= BIT3;
	// ADC10 control register 1
	// ADC10 clock source select; ADC10SSELx
	// Input Channel (P1.5)
	// ADC10 clock divider
	ADC10CTL1 = INCH_3 + ADC10DIV_3 + ADC10SSEL0+ ADC10SSEL1;

	// ADC10 control register 0
	// Select Internal reference voltage(Vr+ = VCC and Vr- = VSS) using REFON
	// and SREF_0
	// Sample and hold time set to 64 × ADC10CLKs using ADC10SHT_3
	// Enable the ADC10 core using ADC10ON bit
	// Enable interrupt
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;

	// Input channel 5(INCH_5) was selected above
	// These bits enable the corresponding pin for analog input. BIT0
	// corresponds to A0, BIT1 corresponds to A1, etc.
	ADC10AE0 |= BIT3;
}

// Timer A0 interrupt service routine
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

	// Add Offset to CCR0
	CCR0 += 30000;

	// Increase timer1 value
	timer++;

	if (timer >= counter) {
		ADC10CTL0 |= ENC + ADC10SC;

		while ((ADC10CTL1 & ADC10BUSY) != 0); //Wait for conversion to finish
		__delay_cycles(50);
		while ((UCA0STAT & UCBUSY) != 0); //Wait for transmit completion
		temp = (ADC10MEM*0.003417)/10;
		UCA0TXBUF = temp * 1000;
		while ((UCA0STAT & UCBUSY) != 0); //Wait for transmit completion

		timer = 0;
	}
}
