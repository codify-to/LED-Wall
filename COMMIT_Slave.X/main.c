/* 
 * File:   main.c
 * Author: Lucas Dupin | Fernando França | Fernando Eid
 *
 * Created on 16 de Junho de 2013, 20:46
 */

#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <plib.h>
#include <p32xxxx.h>
//#include "uart.h"
#include "SPIInit.h"

// Pic initialization bits
#include "configBits.c"

 /*************************
LED Structure and control
*************************/
/*
	4 boards, 
	64 leds. 
*/
#define BOARDS 4
#define LEDS_PER_BOARD 64
unsigned int ledList[BOARDS*LEDS_PER_BOARD];
unsigned char colors_offset[4] = {0, 8, 16, 24};
unsigned char pixel[4][4] = {
        {38, 39, 36, 37},
	{6,   7,  4,  5},
        {34, 35, 32, 33},
        {2,   3,  0,  1}
};
unsigned int bozo;
char uart_buffer[32];
int testBuff = 32;

/*
 * 
 */

void __attribute__ (( interrupt(ipl2),vector(_TIMER_5_IRQ))) T5Handler( void)
{
	LATCbits.LATC1 = !LATCbits.LATC1;

	IFS0bits.T5IF = 0;			// Clear irq flag
}

void initializeTimer(){
	T4CONbits.ON = 0; 		/*Turn off timer*/
	T4CONbits.TCS = 0;		/*Select Internal Clock Source*/
	T4CONbits.TGATE = 0; 	/*Non Gated Mode*/
	T4CONbits.T32 = 1;      /* 32 bit timer (4&5 together) */
	T4CONbits.SIDL = 1;		/*Timer OFF in IDLE mode*/
	T4CONbits.TCKPS = 3;	/* 1:256 Prescaler*/
	TMR4 = 0x0; 		   		/*Cleans the timer*/
	PR4 = (PBFCY/80);		/*Interrupts each 100ms*/
	T4CONbits.ON = 1; 		/*Turn ON timers 4/5*/
	IFS0bits.T5IF=0;		/*Cleans the interruption flag*/
}


void turnOnLed(int board, int x, int y, int color)
{
    unsigned int p = pixel[y][x] + colors_offset[color];;
    
    if (board == 0){
        LATB = ((0x01 << (p % 8))) | ((0x01 << (p / 8)) << 8);
    } else if (board == 1){
        LATA = (0x01 << (p / 8));
        LATF = ((0x01 << (p % 8)));
    } else if (board == 2){
        LATD = ((0x01 << (p % 8))) | ((0x01 << (p / 8)) << 8);
    } else {
        LATE = ((0x01 << (p % 8)));
        LATG = (((0x01 << (p / 8)) & 0xf0) << 8) | ((0x01 << (p / 8)) & 0x0f);
    }
    
    // Delay
    p = 0;
    while(++p < 1000){}
}

unsigned int nBytes;
unsigned int spiData[8];
unsigned int recvBytes;


unsigned int b; // board
unsigned int x; // column
unsigned int y; // row
unsigned int color; // 0 -> green, 1 -> white, 2 -> red, 3 -> blue
unsigned int i;
unsigned int j;

void main()
{

	/* Initialize GPIO ports */

	DDPCONbits.JTAGEN=0; //Disables J-TAG
	AD1PCFG=0xffff; 	//Disables I/O Multiplexing with ADC

	LATA=0;LATB=0;LATC=0;LATD=0;LATE=0;LATF=0;LATG=0;	//Cleans LAT registers

	/*		FEDCBA9876543210*/
	TRISA=0b0000000000000000;
	PORTA=0b0000000000000000;
	/*		FEDCBA9876543210*/
	TRISB=0b0000000000000000;
	PORTB=0b0000000000000000;
	/*		FEDCBA9876543210*/
	TRISC=0b0000000000000000;
	PORTC=0b0000000000000000;
	/*      FEDCBA9876543210*/
	TRISD=0b0000000000000000;
	PORTD=0b0000000000000000;
	/*		FEDCBA9876543210*/
	TRISE=0b0000000000000000;
	PORTE=0b0000000000000000;
	/*		FEDCBA9876543210*/
	TRISF=0b0000000000000000;
	PORTF=0b0000000000000000;
	/*		FEDCBA9876543210*/
	TRISG=0b0000001011000000; // SPI configuration
	PORTG=0b0000000000000000;

	TRISGbits.TRISG8 = 1; // SDO that should be 0

	/*---------------Inits 32-bit Timer 4/5-------------------------------- */

	CheKseg0CacheOn(); //Supercharged mode for PIC32
	SYSTEMConfig(FCY, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

	initializeTimer();

	// Initialize serial communication
//	uart_init();

	// Initialize SPI communication
	SpiInitDevice(SPI_CHANNEL2, 0, 1, 0);	// initialize the SPI channel 2 as slave, frame slave
	
	IPC5bits.T5IP = 2;		/*Timer 5 Priority Level*/
 	INTEnableSystemMultiVectoredInt(); /*Enable multi vectored interruption mode*/
	IEC0bits.T5IE=1;		/*Enables T5 interrupt*/

	sprintf(uart_buffer, "I'm alive\n\r");
//	SendDataBuffer(uart_buffer, 11);


        while(1)
        {

            for (y = 0; y < 4; ++y)
            for (x = 0; x < 4; ++x)
            for (color = 0; color < 4; color++)//TODO ++color)
            for (b = 0; b < BOARDS; ++b)
            {

                turnOnLed(b, x, y, color);

                i = 0;
                while(++i < 100000){}
            }





//            if (SPI2STATbits.SPIRBF)
//            {
//            	SPI2STATbits.SPIRBF = 0;
//                IFS1bits.SPI2RXIF = 0;
//
//            	nBytes = SpiChnGetC(2);
//            	recvBytes = 0;
//            	while(recvBytes < nBytes)
//            	{
//            		while(!SPI2STATbits.SPIRBF)
//            			;
//            		SPI2STATbits.SPIRBF = 0;
//            		spiData[recvBytes] = SpiChnGetC(2);
//            		recvBytes++;
//            	}
//            	sprintf(uart_buffer, "data: 0x%2.2x, 0x%2.2x", spiData[0], spiData[1]);
//            	SendDataBuffer(uart_buffer, 16);
//            }
            
        }
    
}

