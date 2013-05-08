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


char mensagem_lcd[17];		//Buffer para formatar os dados a mostrar no LCD
// unsigned char SPI_Data[64];
unsigned int transmissoes = 0;
unsigned char update_lcd = 1;

void interrupt receiveData(void){

	if(PIR1bits.SSP1IF)
	{
			transmissoes ++;
			PORTBbits.RB1 = 1;

			//Check how many bytes we'll receive
			unsigned char totalBytes = SSP1BUF;
			SSP1BUF = 0;

			// only 2 bytes per led
			unsigned char ledData[2];

			//Wait untill all bytes were received
			int i = 0;
			while(i < totalBytes)
			{
				//Esperamos a chegada do próximo byte
				while(!SSP1STATbits.BF);

				//Fazemos a leitura para dentro do buffer
				ledData[i] = SSP1BUF;
				SSP1BUF = 0;

				i++;
			}
			i=0;

			// int color = ledData[1] & 0b11;
			int board = ledData[0] >> 6 & 0b11;
			int x = ledData[0] >> 4 & 0b11;
			int y = ledData[0] >> 2 & 0b11;
			// ledIntensity[board][x*y] = {.x= x, .y= y, .color= color, .intensity= ledData[1]};
			led* l = &ledList[board][x*y];
			l->x = x;
			l->y = y;
			l->color = ledData[0] & 0b11;
			l->intensity = ledData[1];

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
			sprintf(mensagem_lcd, "T=%d  ", transmissoes);  //Formata mensagem no buffer de dados
			mostra_lcd_buff(1,1, mensagem_lcd,16);

			// sprintf(mensagem_lcd, "0x%X,%X,%X,%X,%X       ", SPI_Data[0], SPI_Data[1], SPI_Data[2], SPI_Data[3], SPI_Data[4]);  //Formata mensagem no buffer de dados
			// mostra_lcd_buff(2,1, mensagem_lcd,16);

			update_lcd=0;
		}

		// Update LED lighting
		// for (int b = 0; b < 4; ++b)
		// 	for (unsigned int l = 0; l < 64; ++l)
		// 	{
		// 		led currLed = ledList[b][l];

		// 		//PORTH = 0x00; //GND
		// 		//PORTJ = 0xff; //VCC
		// 		sprintf(mensagem_lcd, "T=%d  ", l);  //Formata mensagem no buffer de dados
		// 		mostra_lcd_buff(1,1, mensagem_lcd,16);

		// 		PORTH = l >> 8;
		// 		PORTJ = l & 0xff;
		// 		Delay10KTCYx(100);
		// 	}
		// for (unsigned int color = 0; color < 4; ++color)
		// for (unsigned int y = 0; y < 4; ++y)	
		// for (unsigned int x = 0; x < 4; ++x)

		// vermelho, verde, azul, branco
		// primeiro coluna par
		// depois coluna impar
		// 2 em 2 linhas

		// 0, 32,  8, 40
		// 2, 34,  10, 42
		// 16  38 24 56
		// 18, 40, 26, 58

		unsigned char pixel[4][4] = {
			{0,  32,  8, 40},
			{2,  34, 10, 42},
			{16, 48, 24, 56},
			{18, 50, 26, 58}
		};

		unsigned char colors_offset[4] = {0, 1, 4, 5};

		for (unsigned int color = 0; color < 4; ++color)
		for (unsigned int y = 0; y < 4; ++y)	
		for (unsigned int x = 0; x < 4; ++x)
		{
			unsigned int i = pixel[y][x] + colors_offset[color];

			PORTH = ~(0x01 << (i % 8)); //GROUND
			PORTJ = 0x01 << (i / 8); //1 << j; // VCC
			Delay10KTCYx(20);
		}

		// for (unsigned int i = 2; i < 64; i+=8)
		// {
		// 	//PORTH = 0x00; //GND - color
		// 	//PORTJ = 0xff; //VCC - pixel

  //                           sprintf(mensagem_lcd, "T=%d         ", i);  //Formata mensagem no buffer de dados
  //                           mostra_lcd_buff(1,1, mensagem_lcd,16);

		// 	PORTH = ~(0x01 << (i % 8)); //GROUND
		// 	PORTJ = 0x01 << (i / 8); //1 << j; // VCC
		// 	Delay10KTCYx(255);
  //                       Delay10KTCYx(255);

		// }
	}
}