#include <p18f4550.h>		//Defini��o do PIC
#include <delays.h>
#include <spi.h>
#include <string.h>
#include <stdio.h>
#include "LCD_16x2_Pic18.h"	//Inclus�o do m�dulo para controle do LCD

//Aqui o PIC � configurado para iniciar com osiclador interno (1Mhz).
//Ao inciar, ele acelera para 8Mhz
#pragma config FOSC = INTOSC_HS		//Oscilador interno pro CPU, HS pro USB
#pragma config CPUDIV = OSC1_PLL2	//CPU clock vem do PLL de 96 Mhz / 2 = 48Mhz
#pragma config PLLDIV = 5			//Divis�o de 5 para entrar no PLL, 20Mhz / 5 = 4Mhz
#pragma config USBDIV=2				//Clock USB derivado do PLL / 2 = 48Mhz
#pragma config LPT1OSC = OFF		//Modo High Power para o oscilador secund�rio
#pragma config MCLRE = ON			//Master Clear Habilitado

#pragma config IESO = ON			//Troca entre fontes de clock habilitada
#pragma config FCMEN = OFF			//Failsafe clock monitor desabilitado
#pragma config WDT = OFF 			//Watchdog timer desligado
#pragma config PWRT = ON			//Power up timer habilitado
#pragma config BOR = OFF			//Brown out reset desligado
#pragma config PBADEN = OFF			//Pinos RB0, 1, 2 3 e 4 como I/O digital (ADC desconectado)
#pragma config LVP = OFF			//Single supply ICSP programming desligado
#pragma config VREGEN = ON			//Regulador de tens�o USB ligado

char mensagem_lcd[17];		//Buffer para formatar os dados a mostrar no LCD

void main ()				//Fun��o prim�ria
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

	//Dire��o dos ports
	//		76543210
	TRISA=0b00000000;
	TRISB=0b00000001;	//RB0 SDI
	TRISC=0b00000000;
	TRISD=0b00000000;
	TRISE=0b00000100;	//RE2 bot�o teste

	// Inicia o LCD
	lcd_start();	
	
	// Desliga o SPI
	CloseSPI();	

	//Abre SPI. 500Khz, Active High, Leitura no meio do per�odo
	OpenSPI(SPI_FOSC_4,MODE_00,SMPMID);	
	
	sprintf(mensagem_lcd, "    Bom dia! 6  ");  //Formata mensagem no buffer de dados
	mostra_lcd_buff(1,1, mensagem_lcd,16);			//Exibe buffer de dados na tela

	
	while(1)
	{
		//Se apertou o bot�o...
		if(!PORTEbits.RE2)
		{
			//Seleciona Slave
			LATAbits.LATA0 = 1;

			//Primeiro byte informa quantos bytes ser�o enviados.
			//N�o podemos mandar MAIS ou MENOS do que o especificado
			//se n�o o slave trava no loop de recep��o.
			while(WriteSPI(4));				

			//Demais bytes...
			while(WriteSPI(0xA8));			
			while(WriteSPI(0xF3));				
			while(WriteSPI(0xD1));			
			while(WriteSPI(0xE5));			
			//while(WriteSPI(0xC9));

			//Desliga Slave
			LATAbits.LATA0 = 0;	

			//Delay para a pr�xima transmiss�o (n�o � necess�rio).
			//Delay10KTCYx(10);	
		}
		else	//Se n�o...
		{
			//Pisca a porra do led
			Delay10KTCYx(10);
			PORTDbits.RD0=~PORTDbits.RD0;
		}
			
	//Fim do loop infinito
	}
//Fim da fun��o prim�ria
}

/*	------------------------------------------- GARBAGE -----------------------------------------------

				
				while(WriteSPI(0xF5));				//send initial charecter to use the same as flag at slave side and send it till successful transmision
				while( SPI_Slave!= 0xF0 ) 		//Enquanto o slave n�o retornar o dado
				{
					SPI_Slave = ReadSPI();		//Verificamos se o slave j� retornou
				}

				putsSPI(SPI_Send);					//send the string of data to be sent to slave
				


*/