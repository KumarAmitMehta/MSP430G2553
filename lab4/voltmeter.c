//******************************************************************************
// Filename: multimeter.c
// Read analog voltage from the Voltage-divider circuit and convert it to
// corresponding digital value and display that value over UART to Realterm
// terminal running on windows PC.
//
// Steps: 
// - Configure CPU and Watchdog
// - Configure DCO to work at 1 MHz
// - Configure UART Ports
// - Configure ADC control register
// - Start sampling and conversion using ECN and ADC10OSC
// - Conversion result, stored in ADC10MEM
// - Use formula as ADC10MEM/28
// - Use UCA0TXBUF to send voltage measure over UART to PC once per second
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sr	Vin	R1(K Ohm)	R2(K ohm)	Vout	UART(Vout Displayed)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//1	3.5	1.5		4.7		0.8467741935	8
// 2	3.5	3.9		4.7		1.5872093023	16
// 3	3.5	8.2		4.7		2.2248062016	23
// 4	3.5	10		4.7		2.380952381	24
// 5	3.5	15		4.7		2.6649746193	27
// 6	3.5	33		4.7		3.0636604775	31
// 7	3.5	39		4.7		3.1235697941	32
// 8	3.5	47		4.7		3.1818181818	33
// 9	3.5	56		4.7		3.2289950577	33
// 10	3.5	68		4.7		3.2737276479	34
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
unsigned int i, var;

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
    P1SEL |= BIT5;
    // ADC10 control register 1
    // ADC10 clock source select; ADC10SSELx
    // Input Channel (P1.5)
    // ADC10 clock divider
    ADC10CTL1 = INCH_5 + ADC10DIV_3 + ADC10SSEL0+ ADC10SSEL1;
    
    // ADC10 control register 0
    // Select Internal reference voltage(Vr+ = VCC and Vr- = VSS) using REFON
    // and SREF_0
    // Sample and hold time set to 64 × ADC10CLKs using ADC10SHT_3
    // Enable the ADC10 core using ADC10ON bit
    // Enable interrupt
    ADC10CTL0 = SREF_0 + ADC10SHT_3 +  REFON + ADC10ON + ADC10IE;
    
    // Input channel 5(INCH_5) was selected above
    // These bits enable the corresponding pin for analog input. BIT0 
    // corresponds to A0, BIT1 corresponds to A1, etc.
    ADC10AE0 |= BIT5;
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
        ADC10CTL0 |= ENC + ADC10SC;
        while ((ADC10CTL1 & ADC10BUSY) != 0); //Wait for conversion to finish
        // XXX: For every 0.1 volt change the ADC10MEM value changed by approximately
	// 28. The values are shown over the UART as mentioned in the chart above.
	var = (ADC10MEM / 28); 
        UCA0TXBUF = var; 
        while ((UCA0STAT & UCBUSY) != 0); //Wait for transmit completion
        timer = 0;
    }
}
