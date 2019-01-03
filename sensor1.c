#include <msp430.h> 
#include <stdbool.h>
#include "utils.h"
//Version1.0.2
#define TRUE 1
#define FALSE 0


void start();
void stop();
int writeI2C();
int ackOrNack();
void RTCInit();
unsigned int bin2bcd (unsigned int value);
unsigned int bcd2bin (unsigned int value);
int getTimeI2C();
int i2cInit();
void i2cTransmit(unsigned int data[]);
void getTime(unsigned char *array);
void setupCommI2C();
void setTime();

typedef unsigned char  uint8_t;
typdef struct {

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t weekDay;
	uint8_t date;
	uint8_t month;
	uint8_t year;

}rtc;


// [ address 00h, rtc.sec, rtc.min, rtc.hour, rtc weekday, rtc date, rtc month, rtc year]
uint8_t data[TOTAL_BYTES];	
unsigned int recData[ARRAY_LENGTH]; //{seconds, minutes, hours, day, date, month, year}
unsigned int convertedData[ARRAY_LENGTH];
unsigned int maskData[ARRAY_LENGTH] = {0x7F,0x7F,0x3F,0x7,0x3F,0x1F,0xFF};
unsigned char whatTimeIsIt[8];
//_Bool test;
int test;

int main(void) {
	// Stop watchdog timer
      WDTCTL = WDTPW + WDTHOLD;
	  
    // Configure the clock module - MCLK = 1MHz
      DCOCTL = 0;
      BCSCTL1 = CALBC1_1MHZ;
      DCOCTL = CALDCO_1MHZ;

    // Configure P1.6 and P1.7 for I2C
      P1SEL  |= BIT6 + BIT7;
      P1SEL2 |= BIT6 + BIT7;

    //Global interrupt enable
      __bis_SR_register(GIE);
    // Initialize i2c
     if(!i2cInit()) {
    	 test = 1;
     }
     else {
    	 test = 0;
     }
	//  test = getTimeI2C();

    // Set Init Values
      setTime();

    // Initialize device
      RTCInit();

      i2cTransmit(data);

     for(i = 0; i < ARRAY_LENGTH; i++) {

     	testdata[i] = bcd2bin(testdata[i] & maskData[i]);  //convert data

     }

    // Read data
      while(1) {

    //	getTime(&whatTimeIsIt[0]);//

      }

	return 0;
}


void setTime() {
//Slave,register Address, init data
	data[0] = REG_SECONDS;        //Address of sec
	data[1] = rtc.sec = 1;
	data[2] = rtc.min = 52;
	data[3] = rtc.hour = 8;
	data[4] = rtc.date = 1;      // Jan, 1, 19
	data[5] = rtc.month = 1;
	data[6] = rtc.year = 19;
	data[7] = rtc.weekDay = 1;	 // mon-fri
}
int i2cInit() {

	 UCB0CTL1 |= UCSWRST;                     //  USCI_B0 reset enabled config USCI
	 UCB0CTL0 |= UCMODE_3 | UCMST | UCSYNC;   // I2C mode, Master mode, sync, Sending, SMCLK set UCTR fotr W
	 UCB0BR0 = 10;							  // Configure baud rate registers for 100kHz
	 UCB0BR1 = 0;							  //  SMCLK = 1MHz
	 UCB0CTL1 = UCSSEL_2 + UCSWRST;			  // Get clock from SMCLK

	 return 0;
}

void RTCInit() {
	int error;
	
	 UCB0I2CSA = I2C_ADDRESS;           		  // Slave address
	 UCB0CTL1 &= ~UCSWRST;					     // Clear reset

}

void i2cTransmit(uint8_t data[]) {
	 int error, testA, testB;
	 unsigned char testdata[9];
	 int totalBytes = TOTAL_BYTES;
	 int i;

	  UCB0CTL1 |= UCTR | UCTXSTT;

		//send slave address
		  while ((UCB0CTL1 & UCTXSTT) && ((IFG2 & UCB0TXIFG) == 0));

		  error = ackOrNack();
		  IFG2 &= ~UCB0TXIFG;
		  if (!error) {
			  testA = 1;
		  }
		  else {
			  testA = 0;
		  }

	    //IFG2 &= ~UCB0TXIFG;
		  i = 0;
		  //send register address + data
		  while(error == FALSE && totalBytes > 0) {
			  UCB0TXBUF = data[i]; //REG_SECONDS, Init, Data
			  testdata[i] = data[i];
			  i++;
			  while((IFG2 & UCB0TXIFG) == 0);  //UCB0TXIFG is set when UCB0TXBUF is empty
			  error = ackOrNack();

			if (!error) {
				testB = 2;
			}
			else {
				testB = 3;
			}
			IFG2 &= ~UCB0TXIFG;
			totalBytes--;
		  }
		
		for(i = 0; i < ARRAY_LENGTH; i++) {

			testdata[i] = bcd2bin(testdata[i] & maskData[i]);  //convert data
		}

		while((UCB0STAT & BUSY)!= 0);
		 	 UCB0CTL1 |= UCTXSTP ; //stop();


}

void getTime(unsigned char *array) {
	 int error;
	 int i;

	//Set up for read
	 UCB0CTL1 &= ~UCTR;
	 UCB0CTL1 |=  UCTXSTT;

	 //Wait for start to be sent
	 while(UCB0CTL1 & UCTXSTT){}     //wait for start to be sent

	 error = ackOrNack();

	 if(error == FALSE) { 					//nacK not encountered

		for(i = 0; i < ARRAY_LENGTH; i++) {

			while(!(IFG2 & UCB0RXIFG)){}   //wait till rec char
		 	 *array = UCB0RXBUF;
		 	 *array++;
	    }
		UCB0CTL1 |= UCTXSTP;//stop();
	 }

	/*	for(i = 0; i < ARRAY_LENGTH; i++) {

			convertedData[i] = bcd2bin(recData[i] & maskData[i]);  //convert data
		}*/
}

void setupCommI2C() {

	UCB0I2CSA = I2C_ADDRESS;           			 // Slave address
	UCB0CTL1 &= ~UCSWRST;					     // Clear reset

}

int getTimeI2C() {
	  int i = 0;
	  int error = -1;

	  UCB0CTL1 |= UCSWRST;                      // Software reset enabled
	  UCB0CTL0 |= UCMODE_3 | UCMST | UCSYNC;    // I2C mode, Master mode, sync, Sending, SMCLK set UCTR fotr W
	  UCB0BR0 = 10;
	  UCB0BR1 = 0;
	  UCB0CTL1 = UCSSEL_2 + UCSWRST;
	  UCB0I2CSA = I2C_ADDRESS;           		// Slave address
	  UCB0CTL1 &= ~UCSWRST;					    // Clear reset
	
	//start();
	  UCB0CTL1 |= UCTR | UCTXSTT;

		//send address
		  while ((UCB0CTL1 & UCTXSTT) && ((IFG2 & UCB0TXIFG) == 0));
		//send register address
		  error = ackOrNack();
		  IFG2 &= ~UCB0TXIFG;
		  UCB0TXBUF = REG_MINUTES; //REG_SECONDS;
		
		  while((IFG2 & UCB0TXIFG) == 0);  //UCB0TXIFG is set when UCB0TXBUF is empty
		  error = ackOrNack();

		//stop();
		//while(UCB0STAT & UCTXSTP){}  //wait for stp - send stp

		UCB0CTL1 &= ~UCTR;				

		//Get ready to read data
		UCB0CTL1 |= UCTXSTT;

		//start();
		while(UCB0CTL1 & UCTXSTT){}     //wait for start to be sent

		for(i = 0; i < ARRAY_LENGTH; i++) {
			while(!(IFG2 & UCB0RXIFG)){}  
			recData[i] = UCB0RXBUF;
		}

		UCB0CTL1 |= UCTXSTP ;//stop();
		while(UCB0STAT & UCTXSTP){}  //wait for stp - send stp

		for(i = 0; i < ARRAY_LENGTH; i++) {
	//		convertedData[i] = bcd2bin(recData[i] & maskData[i]);  //convert data
		}

		return 0;

}


unsigned int bin2bcd(unsigned int value) {
	return value + 6 * (value / 10);
}

unsigned int bcd2bin(unsigned int value) {
	return value - 6 * (value >> 4);
}

// If slave ack the address the UCTXSTT is cleared
int ackOrNack() {
	int nack = FALSE;
		//both should work comment one out
		if(UCB0STAT & UCNACKIFG) {
			nack = TRUE;
			UCB0CTL1 |= UCTXSTP; //STOP TRANSMISSION
			UCB0STAT  &= ~UCNACKIFG;  //Clear NACK Flag
		}
		return nack;
}

void start() {
        // UCTR = 1 UCTXSTT = 1
	//UCB0CTL1 |= 0b00000010 //set as transmitter and send start condition
	// UCB0CTL1 |= UCTR | UCTXSTT;
}

void stop() {
	//UCB0CTL1 |= 0b00000100 //Send stop condition
	UCB0CTL1 |= UCTXSTP; //Send stop condition
}

//transmit slave address
//send data bytes
//ack or nack by slave after each byte
//msb transferred


