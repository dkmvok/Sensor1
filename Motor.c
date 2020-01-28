#include <msp430.h> 
#include "uart.h"
#include "motor.h"

/*
 * main.c
 */

typedef enum { motorOff, motorOn } motorState;
typedef enum { false = 0, true = !false } bool;


int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer to prevent time out reset

    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;

    P1OUT &= 0x00;	// Reset
    P1DIR &= 0x00;
    P2OUT &= 0x00;
    P2DIR &= 0x00;

    P1DIR |= BIT0 + BIT1 + BIT2 + BIT4 + BIT6;	//Set only numbered ports as output
    P1REN |= BIT3;
    P1OUT |= BIT3;

    P1IE |= BIT3;	//Enable interrupt
    P1IES |= BIT3;
    P1IFG &= ~BIT3;

    P2OUT = 0x00;
    P2DIR |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5);


    TA0CCR0 = Counter;					//Set up terminal count
    TA0CCTL0 |= CCIE;					//Enable Interrupts
    TA0CTL = TASSEL_2 + MC_1 + TACLR;	
    stepCount = 0;
    stepArrayPtr = &stepArrayCW[0];
    comleteCycle = false;

    _enable_interrupt();

    //_BIS_SR(GIE);
    while(1) {
    	if( stepCount == 50) {
    		stepCount = 0;
    	}
    	if(completeCycle) {
    		P1OUT ^= BIT6;
        }
    	TA0CCR0 = Counter;


	return 0;
}



#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {

	char check = 'A'; //Test Point

	if(index < 8) {
		check = 'R';
		P2OUT = (stepArrayPtr->stepArrayCW[index]) & 0x0F;
		index++;
		stepCount++;
		completeCycle = true;
	}
	if(index >= 8) {
		check = 'L';
		stepArrayPtr = &stepArrayCCW[0];
		P2OUT = (stepArrayPtr->stepArrayCCW[index]) & 0x0F;
		index++;
		stepCount++;
	}

	TA0CCTL0 &= ~CCIFG;
	return;
}

