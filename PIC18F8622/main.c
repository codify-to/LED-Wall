#include <p18f8622.h>
#include <delays.h>
#include <spi.h>
#include <string.h>
#include <stdio.h>
#include "LCD_16x2_Pic18.h"	//Inclusão do módulo para controle do LCD

#include "configBits.h"

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int color;
	unsigned char intensity;
} led; 
/*
	4 boards, 
	64 leds. 
*/
led ledList[4][64];
unsigned char colors_offset[4] = {0, 1, 4, 5};
unsigned char pixel[4][4] = {
	{0,  32,  8, 40},
	{2,  34, 10, 42},
	{16, 48, 24, 56},
	{18, 50, 26, 58}
};
void lightUp(led *l, int board);


char mensagem_lcd[17];		//Buffer para formatar os dados a mostrar no LCD
// unsigned char SPI_Data[64];
unsigned int transmissoes = 0;
unsigned char update_lcd = 1;

/***************
Interrupt vars
***************/
// only 2 bytes per led
unsigned char ledData[2];
unsigned char totalBytes;
int recBytes=0;
led *lcdLed;

void interrupt receiveData(void){

	if(PIR1bits.SSP1IF)
	{
		transmissoes ++;
		PORTBbits.RB1 = 1;

		//Check how many bytes we'll receive
		totalBytes = SSP1BUF;
		SSP1BUF = 0;

		//Wait untill all bytes were received
		recBytes = 0;
		while(recBytes < totalBytes)
		{
			//Esperamos a chegada do próximo byte
			while(!SSP1STATbits.BF);

			//Fazemos a leitura para dentro do buffer
			ledData[recBytes] = SSP1BUF;
			SSP1BUF = 0;

			recBytes++;
		}
		recBytes=0;

		// unsigned int color = ledData[1] & 0b11;
		unsigned int board = ledData[0] >> 6 & 0b11;
		unsigned int x = ledData[0] >> 4 & 0b11;
		unsigned int y = ledData[0] >> 2 & 0b11;
		led* l = &ledList[board][x + y*4];
		l->x = x;
		l->y = y;
		l->color = ledData[0] & 0b11;
		l->intensity = ledData[1];

		lcdLed = l;

		update_lcd = 1;
		PORTBbits.RB1 = 0;
		PIR1bits.SSP1IF = 0; 	//Limpa a flag de interrupção
	}
}

void main (void)
{

	//Opera em 8Mhz se oscilador interno for escolhido
	OSCCONbits.IRCF=0b111;	//Acelera pra 8Mhz

	// Desliga o ADC do cacete (PORTF)
	ADCON0bits.ADON = 0;
	ADCON1bits.PCFG0 = 1;
	ADCON1bits.PCFG1 = 1;
	ADCON1bits.PCFG2 = 1;
	ADCON1bits.PCFG3 = 1;

	// Desliga o comparador do caralho (PORTF)
	CMCONbits.CM0 = 1;
	CMCONbits.CM1 = 1;
	CMCONbits.CM2 = 1;

	// Setup pin direction
	//  0 -> out
	//	1 -> in
	TRISA=0b00000000;
	TRISB=0b00000000;
	TRISC=0b00011000;	//RC3 = clock in, RC4 = SDI
	TRISD=0b00000000;
	TRISE=0b00000000;
	TRISF=0b00000000;
	TRISG=0b00000000;
	TRISH=0b00000000;
	TRISJ=0b00000000;

	// Reset values
	PORTA=0x00;
	PORTB=0x00;
	PORTC=0x00;
	PORTD=0x00;
	PORTE=0x00;
	PORTF=0x00;
	PORTG=0x00;
	PORTH=0x00; // GND
	// Turn all leds on for startup debugging
	PORTJ=0xff; // HIGH

	INTCONbits.GIE=1;   //Habilita as interrupções
    PEIE = 1;           //Habilita a interrupção de todos os periféricos

	lcd_start();	// Inicia o LCD

	// Cleanup default memory values
	// so, they will start off
	for (int board = 0; board < 4; ++board)
	for (int i = 0; i < 64; ++i)
	{
		led *l = &ledList[board][i];
		l->intensity = 0;
		l->x = i % 4;
		l->y = i / 4;
	}
	lcdLed = &ledList[0];

	//Desliga SPI
	CloseSPI();

	//Religa o SPI em modo Slave com SS, Active High, Leitura no meio do período
	OpenSPI(SLV_SSON,MODE_00,SMPMID);

	//Habilita interrupção do SPI
	PIR1bits.SSP1IF = 0;	//Limpa a flag de interrupção por precaução
	PIE1bits.SSP1IE = 1;	//Habilita a interrupção

	while(1)
	{

		if(update_lcd)
		{
			sprintf(mensagem_lcd, "T=%d x:%d y:%d c:%d ", transmissoes, lcdLed->x, lcdLed->y, lcdLed->color);  //Formata mensagem no buffer de dados
			mostra_lcd_buff(1,1, mensagem_lcd,16);

			update_lcd=0;
		}

		// for (int board = 0; board < 4; ++board)
		// for (int i = 0; i < 64; ++i)
		// {
		// 	led *l = &ledList[board][i];
		// 	lightUp(l, board);
		// 	// Delay10KTCYx(20);
		// }

		for (unsigned int color = 0; color < 4; ++color)
		for (unsigned int x = 0; x < 4; ++x)
		for (unsigned int y = 0; y < 4; ++y)
		{
			unsigned int p = pixel[y][x] + colors_offset[color];

			PORTH = ~(0x01 << (p % 8)); //GROUND
			PORTJ = 0x01 << (p / 8); //1 << j; // VCC
			// Delay10KTCYx(20);
		}

	}
}

void lightUp(led *l, int board){
	
	unsigned int i = pixel[l->y][l->x] + colors_offset[l->color];
	unsigned int val;
	if (board == 0)
	{
		/****
		 This board has special cases
		 *****/

		// GND
		PORTA = ~(0x01 << (i % 8));
		// VCC
		val = 0x01 << (i / 8);
		if(val == 5){
			PORTC = 6;
		} else {
			PORTB = val; //1 << j; // VCC
		}
	} else if (board == 1)
	{
		// GND
		PORTD = ~(0x01 << (i % 8));
		// VCC
		PORTE = 0x01 << (i / 8);
	} else if (board == 2)
	{
		/****
		 This board has special cases
		 *****/

		// GND
		val = ~(0x01 << (i % 8));
		if(val == 7)
			PORTC = 7;
		else {
			PORTF = val;
		}
		// VCC
		val = 0x01 << (i / 8);
		if (val < 5)
		{
			PORTG = val;
		} else {
			PORTC = val - 5;
		}
		PORTJ = val; // VCC
	} else if (board == 3)
	{
		// GND
		PORTH = ~(0x01 << (i % 8));
		// VCC
		PORTJ = 0x01 << (i / 8);
	}
}