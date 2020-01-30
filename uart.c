#include <msp430.h> 
#include <stdio.h>
#include <stdint.h>
#include "uart.h"
#define ARRAY_SIZE 16
/*
 * main.c
 */
//2400, 4800, 9600

//int ARRAY_SIZE = 10;

const unsigned baudRate = 9600;
static const struct baudRates baudRateTable[] =  {
		{2400, 4800, 9600}

};
char message[ARRAY_SIZE] = 0;

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    BCSCTL = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    uartInit();

    unsigned char = ch;

    print("TestIO\n\r");
    putString((char *)"Testing\n");
    ch = getChar();
    putChar(ch);
    putString((char *)"Input String: =\n");

    	while(1) {

    		getString(message,ARRAY_SIZE);
    		putString(message);


    	}
	
	return 0;
}

void uartInit(void) {

	P1DIR |= BIT0;
	P1OUT |= BIT0;

	P1SEL = BIT1 + BIT2;
	P1SEL2 = BIT1 + BIT2;

	UCA0CTL1 |= UCSSEL_2;
	UCA0BR0 = (baudRate >> 12) & 0XFF;
	UCA0BR1 = (baudRate >> 4) & 0XFF;
	UCA0MCTL = ((baudRate << 4) & 0xF0) | UCOS16;
	UCA0CTL1 &= ~UCSWRT;
	IE2 |= UCA0RXIE;


	//didRecieve = 0;
	//didTransmit = 0;

	return;

}


unsigned char getChar(void) {

	unsigned char rxChar;

	while(!(IFG2 & UCA0RXIFG));
		rxChar = UCA0RXBUF;

		return rxChar;

}


void getString(char *rxMessage, int length) {

	unsigned int i = 0;

	while(i < length) {

		*rxMessage = getChar();
		if( *rxMessage == '\r') {

				while(i < length) {

					*rxMessage = '\0';
					i++;

				}
				break;
	   }
	*rxMessage++;

   }
	return;
}

void putChar(unsigned ch) {

	while(!IFG2 & UCA0TXIFG));
		UCA0TXBUF = ch;

	return;

}

void putString(unsigned char *txMessage) {

	unsigned int i = 0;
	unsigned char ch;

	while(*txMessage != '\0'){
		 putChar(*txMessage++);

		if(*txMessage == '\n') {

		     putChar('\r');

		}

	}
	return;
}






