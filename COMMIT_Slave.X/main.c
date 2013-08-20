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
#include "uart.h"
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
    unsigned int p;
    
    if (board == 0){
        p = pixel[y][x] + colors_offset[color];
        LATB = ((0x01 << (p % 8))) | ((0x01 << (p / 8)) << 8);
    } else if (board == 1){

    } else if (board == 2){
        p = pixel[y][x] + colors_offset[color];
        LATD = ((0x01 << (p % 8))) | ((0x01 << (p / 8)) << 8);
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
	uart_init();

	// Initialize SPI communication
	SpiInitDevice(SPI_CHANNEL2, 0, 1, 0);	// initialize the SPI channel 2 as slave, frame slave
	
	IPC5bits.T5IP = 2;		/*Timer 5 Priority Level*/
 	INTEnableSystemMultiVectoredInt(); /*Enable multi vectored interruption mode*/
	IEC0bits.T5IE=1;		/*Enables T5 interrupt*/

	sprintf(uart_buffer, "I'm alive\n\r");
	SendDataBuffer(uart_buffer, 11);


        while(1)
        {
            // Reset all pixels
//            LATDbits.LATD15 = 1;
//            LATDbits.LATD14 = 1;
//            LATDbits.LATD13 = 1;
//            LATDbits.LATD12 = 1;
//            LATDbits.LATD11 = 1;
//            LATDbits.LATD10 = 1;
//            LATDbits.LATD9 = 1;
//            LATDbits.LATD8 = 1;
//            LATDbits.LATD7 = 1;
//            LATDbits.LATD6 = 1;
//            LATDbits.LATD5 = 1;
//            LATDbits.LATD4 = 1;
//            LATDbits.LATD3 = 1;
//            LATDbits.LATD2 = 1;
//            LATDbits.LATD1 = 1;
//            LATDbits.LATD0 = 1;

            // SendDataBuffer(uart_buffer, 7);
//            PORTD = 0xff;
//            LATD = 0b1111111111111111;
//            PORTD = 0xff;
//            LATD = 0b1 << 4;
//            bozo = 0xff;
//            LATD = 0b000001000000010;//bozo; //0b000001000000010;
//            LATD = 0b000000100000001;
//            LATD = 0b010000000100000;
//            LATD = 0b1111111000000001;
//          LATD = 0b0000000000000010;
//            LATD = 0b0000000000000101;
//            LATD = 0b0000000000000110;

            // A direita é ground

//            i = 39+24;
//            LATD = ((0x01 << (i % 8))) | ((0x01 << (i / 8)) << 8); //GROUND

//            for (x = 0; x < 32; x+=8)
//            {
//                i = 36 + x;
//              LATD = ((0x01 << (i % 8))) | ((0x01 << (i / 8)) << 8); //GROUND
//
//              bozo = 0;
//               while(++bozo < 1999999){}
//            }

            
//            for (y = 0; y < 4; ++y)
//            for (x = 0; x < 4; ++x)
//            for (color = 0; color < 4; ++color)
//            {
//                    i = pixel[y][x] + colors_offset[color];
//                    LATD = ((0x01 << (i % 8))) | ((0x01 << (i / 8)) << 8);
//
//                    i = 0;
//                    while(++i < 500000){}
//            }


//            for (color = 0; color < 4; ++color) {
//
//              turnOnLed(0, 0, color);
//              turnOnLed(1, 0, color);
//
//
//              turnOnLed(0, 2, color);
//              turnOnLed(1, 2, color);
//
//
//              turnOnLed(0, 1, color);
//              turnOnLed(0, 3, color);
//
//
//              turnOnLed(2, 0, (color+1)%4);
//              turnOnLed(3, 0, (color+1)%4);
//
//
//              turnOnLed(2, 2, (color+1)%4);
//              turnOnLed(3, 2, (color+1)%4);
//
//
//              turnOnLed(2, 1, (color+1)%4);
//              turnOnLed(2, 3, (color+1)%4);
//
//              LATD = 0b0;
//
//               i = 0;
//                    while(++i < 100000){}
//            }

            for (y = 0; y < 4; ++y)
            for (x = 0; x < 4; ++x)
            for (color = 0; color < 4; ++color)
            for (b = 0; b < BOARDS; ++b)
            {
//                if(color % 2 == 1)
//                {
//                    j = y;
//                }
//                else
//                {
//                   j = 3 - y;
//                }
//
//                if((j + color) % 2 == 0) {
//                    i = x;
//                }
//                else
//                {
//                    i = 3 - x;
//                }

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

