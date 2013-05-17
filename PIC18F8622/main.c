#include <p18f8622.h>
#include <delays.h>
#include <spi.h>
#include <string.h>
#include <stdio.h>
#include "LCD_16x2_Pic18.h"	//Inclusão do módulo para controle do LCD

#include "configBits.h"

/*************************
LED Structure and control
*************************/
/*
	4 boards, 
	64 leds. 
*/
unsigned int ledList[4*64];
unsigned int ledX[4*64];
unsigned int ledY[4*64];
unsigned int ledC[4*64];
unsigned char colors_offset[4] = {0, 1, 4, 5};
unsigned char pixel[4][4] = {
	{0,  32,  8, 40},
	{2,  34, 10, 42},
	{16, 48, 24, 56},
	{18, 50, 26, 58}
};
inline void lightUp(unsigned int address);

/***************************
Interrupt
***************************/
void startTimer0();


/*********
Debug vars
*********/
char mensagem_lcd[17];		//Buffer para formatar os dados a mostrar no LCD
// unsigned char SPI_Data[64];
unsigned int transmissoes = 0;
unsigned char update_lcd = 1;

/***************
Buffer vars
***************/
// only 2 bytes per led
unsigned char ledData[2];
unsigned char totalBytes;
int recBytes=0;
unsigned int i, x, y, c, board, intensity, val;
// 
unsigned int _x;
unsigned int _y;
unsigned int _board;
unsigned int _color;
unsigned int _p;

void interrupt receiveData(void){

	// if(PIR1bits.SSP1IF)
	// {
	// 	PIR1bits.SSP1IF = 0; 	//Limpa a flag de interrupção
	// }

	// 
	if (INTCONbits.TMR0IF)
	{
		for (board = 0; board < 4; ++board)
			for (_color = 0; _color < 4; ++_color)
			for (_x = 0; _x < 4; ++_x)
			for (_y = 0; _y < 4; ++_y)
			{
				_p = pixel[_y][_x] + colors_offset[_color];

				LATH = ~(0x01 << (_p % 8)); //GROUND
				LATJ = 0x01 << (_p / 8); //1 << j; // VCC

				intensity = ledList[
					(_x << 4) +
					(_y << 2) +
					(_color)
				];

				if (board == 0)
				{
					/****
					 This board has special cases
					 *****/

					// GND
					LATA = ~(0x01 << (_p % 8));
					// VCC
					val = 0x01 << (_p / 8);
					if(val == 5){
						LATC = 6;
					} else {
						LATB = val; //1 << j; // VCC
					}
				} else if (board == 1)
				{
					// GND
					LATD = ~(0x01 << (_p % 8));
					// VCC
					LATE = 0x01 << (_p / 8);
				} else if (board == 2)
				{
					/****
					 This board has special cases
					 *****/

					// GND
					val = ~(0x01 << (_p % 8));
					if(val == 7)
						LATC = 7;
					else {
						LATF = val;
					}
					// VCC
					val = 0x01 << (_p / 8);
					if (val < 5)
					{
						LATG = val;
					} else {
						LATC = val - 5;
					}
					LATJ = val; // VCC
				} else if (board == 3)
				{
					// GND
					LATH = ~(0x01 << (_p % 8));
					// VCC
					LATJ = 0x01 << (_p / 8);
				}
				
				// Delay10KTCYx(20);
			}

		LATAbits.LATA0 = ~LATAbits.LATA0;

		INTCONbits.TMR0IF=0;
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
	Delay10KTCYx(200);
	PORTJ=0x00; // HIGH

	INTCONbits.GIE=1;   //Habilita as interrupções
    PEIE = 1;           //Habilita a interrupção de todos os periféricos

	lcd_start();	// Inicia o LCD

	int board, x, y, c;

	// Cleanup default memory values
	// so, they will start off
	for (int i = 0; i < 4*64; ++i){
		ledList[i] = 0;
		ledX[i] = i >> 4 & 0b11;
		ledY[i] = i >> 2 & 0b11;
		ledC[i] = i & 0b11;
	}
	
	//Desliga SPI
	CloseSPI();
	//Religa o SPI em modo Slave com SS, Active High, Leitura no meio do período
	OpenSPI(SLV_SSON,MODE_00,SMPMID);

	//Habilita interrupção do SPI
	PIR1bits.SSP1IF = 0;	//Limpa a flag de interrupção por precaução
	PIE1bits.SSP1IE = 0;//1;	//Habilita a interrupção

	// Enable timer
	startTimer0();

	while(1)
	{

		if(update_lcd)
		{
			sprintf(mensagem_lcd, "T=%d", transmissoes);
			mostra_lcd_buff(1,1, mensagem_lcd,16);

			update_lcd=0;
		}

		if(PIR1bits.SSP1IF){
			PIR1bits.SSP1IF = 0;

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

			ledList[ledData[0]] = ledData[1];
			
			transmissoes++;
			update_lcd=1;

			PORTBbits.RB1 = 0;
		}
		
		// for (int board = 0; board < 4; ++board)
		// for (int i = 0; i < 64; ++i)
		// {
		// 	led *l = &ledList[board][i];
		// 	lightUp(l, board);
		// 	// Delay10KTCYx(20);
		// }

	}
}

inline void lightUp(unsigned int address){

	intensity = ledList[address];
	// if (intensity == 0) return;

	board = address >> 6 & 0b11;
	board = 3;
	x = address >> 4 & 0b11;
	y = address >> 2 & 0b11;
	c = address & 0b11;
	i = pixel[y][x] + colors_offset[c];

	return;

	if (board == 0)
	{
		/****
		 This board has special cases
		 *****/

		// GND
		LATA = ~(0x01 << (i % 8));
		// VCC
		val = 0x01 << (i / 8);
		if(val == 5){
			LATC = 6;
		} else {
			LATB = val; //1 << j; // VCC
		}
	} else if (board == 1)
	{
		// GND
		LATD = ~(0x01 << (i % 8));
		// VCC
		LATE = 0x01 << (i / 8);
	} else if (board == 2)
	{
		/****
		 This board has special cases
		 *****/

		// GND
		val = ~(0x01 << (i % 8));
		if(val == 7)
			LATC = 7;
		else {
			LATF = val;
		}
		// VCC
		val = 0x01 << (i / 8);
		if (val < 5)
		{
			LATG = val;
		} else {
			LATC = val - 5;
		}
		LATJ = val; // VCC
	} else if (board == 3)
	{
		// GND
		LATH = ~(0x01 << (i % 8));
		// VCC
		LATJ = 0x01 << (i / 8);
	}
}

void startTimer0()
{
	// PIC timer initialization
	INTCONbits.TMR0IF=0;	//Limpa a flag de interrupção do Timer 0 (apenas por precaução)
	INTCONbits.TMR0IE=1;	//Habilita interrupção do Timer 0
	INTCON2bits.TMR0IP=1;	//Define alta prioridade para interrupção do Timer 0
	// Timer bits
	T0CONbits.TMR0ON=0;			//Timer 0 desligado para configuração
	T0CONbits.T08BIT=1;			//Seleção entre modo 8(1) e 16 bits(0).
	T0CONbits.T0CS=0;			//Seleção entre clock interno(0) ou transição no pino(1) T0CKI (RA4)
	T0CONbits.T0SE=0;			//Seleção entre resposta à rampa de subida(0) ou descida(1) no pino T0CKI.
	T0CONbits.PSA=0;			//Pre-scaler selecionado(0) ou "bypass"(1), onde o clock é injetado diretamente no timer.
	T0CONbits.T0PS=0b100;		//Divisão do Pre-scaler. Vai de 1:2 (0b000) à 1:256 (0b111);
	T0CONbits.TMR0ON=1;			//Timer 0 ligado!
}