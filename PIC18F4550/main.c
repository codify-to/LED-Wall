#include <p18f4550.h>		//Definicao do PIC
#include <delays.h>
#include <spi.h>
#include <string.h>
#include <stdio.h>
#include "LCD_16x2_Pic18.h"	//Inclusao do modulo para controle do LCD

//Aqui o PIC é configurado para iniciar com osiclador interno (1Mhz).
//Ao inciar, ele acelera para 8Mhz
#pragma config FOSC = INTOSC_HS    //Oscilador interno pro CPU, HS pro USB
#pragma config CPUDIV = OSC1_PLL2  //CPU clock vem do PLL de 96 Mhz / 2 = 48Mhz
#pragma config PLLDIV = 5          //Divisao de 5 para entrar no PLL, 20Mhz / 5 = 4Mhz
#pragma config USBDIV=2            //Clock USB derivado do PLL / 2 = 48Mhz
#pragma config LPT1OSC = OFF       //Modo High Power para o oscilador secundario
#pragma config MCLRE = ON          //Master Clear Habilitado

#pragma config IESO = ON           //Troca entre fontes de clock habilitada
#pragma config FCMEN = OFF         //Failsafe clock monitor desabilitado
#pragma config WDT = OFF           //Watchdog timer desligado
#pragma config PWRT = ON           //Power up timer habilitado
#pragma config BOR = OFF           //Brown out reset desligado
#pragma config PBADEN = OFF        //Pinos RB0, 1, 2 3 e 4 como I/O digital (ADC desconectado)
#pragma config LVP = OFF           //Single supply ICSP programming desligado
#pragma config VREGEN = ON         //Regulador de tensao USB ligado

char mensagem_lcd[16]; //Buffer para formatar os dados a mostrar no LCD

void main ()
{

	//Opera em 8Mhz se oscilador interno for escolhido
	OSCCONbits.IRCF=0b111;	//Acelera pra 8Mhz

	// Desliga o ADC
	ADCON1bits.PCFG=0b1111;

	// Desliga o comparador
	CMCONbits.CM=0b111;

		//Limpeza dos ports:
	//		76543210
	PORTA=0b00000000;
	PORTB=0b00000000;
	PORTC=0b00000000;
	PORTD=0b00000000;
	PORTE=0b00000000;

	//Direcao dos ports
	//		76543210
	TRISA=0b00000000;
	TRISB=0b00000001;	//RB0 SDI
	TRISC=0b00000000;
	TRISD=0b00000000;
	TRISE=0b00000100;	//RE2 botao teste

	// Inicia o LCD
	lcd_start();

	// Desliga o SPI
	CloseSPI();

	// Abre SPI. 500Khz, Active High, Leitura no meio do periodo
	OpenSPI(SPI_FOSC_4,MODE_00,SMPMID);

	sprintf(mensagem_lcd, "Master          ");  //Formata mensagem no buffer de dados
	mostra_lcd_buff(1,1, mensagem_lcd,16);      //Exibe buffer de dados na tela

	unsigned char board = 0;
	unsigned char x = 0;
	unsigned char y = 0;
	unsigned char color = 0;
	unsigned char led_byte;

	while(1)
	{
		//Se apertou o botão
		if(!PORTEbits.RE2)
		{
                        sprintf(mensagem_lcd, "Botao  loop     ");  //Formata mensagem no buffer de dados
                        mostra_lcd_buff(1,1, mensagem_lcd,16);
			//Seleciona Slave
						//LAT - 1 ciclo write only
						//PORT - 2 ciclos, read-write.
			LATAbits.LATA0 = 1;

			//Primeiro byte informa quantos bytes serao enviados.
			//Nao podemos mandar MAIS ou MENOS do que o especificado
			//se nao o slave trava no loop de recepcaoo.

//                        while(WriteSPI(2)); // byte 0: number of bytes to be received by slave
//
//			while(WriteSPI(0x22)); // byte 1: led address
//                        while(WriteSPI(0x7F)); // byte 2: intensity (sample 50%)


			for (board = 0; board < 4; board++)
			{
				for(y = 0; y < 4; y++)
				{
					for(x = 0; x < 4; x++)
					{
						for (color = 0; color < 4; ++color)
						{
							while(WriteSPI(2)); // byte 0: number of bytes to be received by slave
							led_byte = (board << 6) | (x << 4) | (y << 2) | color;
							while(WriteSPI(led_byte)); // byte 1: led address
							while(WriteSPI(0x7F)); // byte 2: intensity (sample 50%)

							// Test delay
							Delay10KTCYx(1);
						}
					}
				}
			}

			//Desliga Slave
			LATAbits.LATA0 = 0;

			//Delay para a proxima transmissao
			Delay10KTCYx(10);
		}
		else
		{
			//Pisca a porra do led
			Delay10KTCYx(10);
			PORTDbits.RD0=~PORTDbits.RD0;
		}
	}
}
