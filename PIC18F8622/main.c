#include <p18f8622.h>
#include <delays.h>
#include <spi.h>
#include <string.h>
#include <stdio.h>

#include "LCD_16x2_Pic18.h"	//Inclusão do módulo para controle do LCD

char mensagem_lcd[17];		//Buffer para formatar os dados a mostrar no LCD
unsigned char SPI_Recv[21];



unsigned char nBytes_SPI = 0;
unsigned char SPI_Data[64];
unsigned int transmissoes = 0;
unsigned char update_lcd = 1;
int i=0;


void ISR_alta_prioridade(void);	//protótipo da função de interrupção
void Inicia_Timer0();			// Protótipo da função que incia o Timer 0

// ---------------------------- ROTINA DE INTERRUPÇÃO ---------------------------//

#pragma code int_alta=0x08
void int_alta(void)
{
	_asm GOTO ISR_alta_prioridade _endasm
}
#pragma code

#pragma interrupt ISR_alta_prioridade 
void ISR_alta_prioridade(void)
{
	//PORTJbits.RJ7 = ~PORTJbits.RJ7;

 	//Confirma a fonte da interrupção (SPI)
	if(PIR1bits.SSP1IF) 
	{
		transmissoes ++;
		PORTBbits.RB1 = 1;

		//Pega o primeiro byte que representa o número de bytes que serão transmitidos em seguida
		nBytes_SPI = SSP1BUF;	
		SSP1BUF = 0;			
								
		//Fica preso no loop até recebermos todos os bytes
		while(i < nBytes_SPI)
		{
			//Esperamos a chegada do próximo byte
			while(!SSP1STATbits.BF); 

			//Fazemos a leitura para dentro do buffer
			SPI_Data[i] = SSP1BUF; 
			SSP1BUF = 0;
								
			i++;
		} 
		i=0;	
	}

	update_lcd = 1;
	PORTBbits.RB1 = 0;
	PIR1bits.SSP1IF = 0; 	//Limpa a flag de interrupção

	//INTCONbits.TMR0IF=0; //Timer 0 interruption flag
}

// ---------------------------- FIM ROTINA DE INTERRUPÇÃO ---------------------------//

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
	
	//Acerta o TRIS para entradas e saídas de forma organizada no hardware
	
		//0b76543210
	TRISA=0b00000000;
	TRISB=0b00000000;
	TRISC=0b00011000;	//RC3 = Entrada de clock, RC4 = SDI
	TRISD=0b00000000;
	TRISE=0b00000000;
	TRISF=0b00000000;
	TRISG=0b00000000;
	TRISH=0b00000000;
	TRISJ=0b00000000;
	
	//Inicializa tudo com 0
	
	PORTA=0x00;
	PORTB=0x00;
	PORTC=0x00;
	PORTD=0x00;
	PORTE=0x00;
	PORTF=0x00;
	PORTG=0x00;
	PORTH=0x00;
	PORTJ=0xff;
	
	INTCONbits.GIEH=1;		//Habilita as interrupções de alta prioridade
	INTCONbits.GIEL=1;		//Habilita as interrupções de baixa prioridade

	lcd_start();	// Inicia o LCD
	
	//Desliga SPI
	CloseSPI();	
	
	//Religa o SPI em modo Slave com SS, Active High, Leitura no meio do período
	OpenSPI(SLV_SSON,MODE_00,SMPMID);	

	//Habilita interrupção do SPI
	IPR1bits.SSP1IP = 1;	//Seta interrupção de alta prioridade
	PIR1bits.SSP1IF = 0;	//Limpa a flag de interrupção por precaução
	PIE1bits.SSP1IE = 1;	//Habilita a interrupção

	//Pertinentes ao Timer 0
	//INTCONbits.TMR0IF=0;	//Limpa a flag de interrupção do Timer 0 (apenas por precaução)
	//INTCONbits.TMR0IE=1;	//Habilita interrupção do Timer 0
	//INTCON2bits.TMR0IP=1;	//Define alta prioridade para interrupção do Timer 0
	//Inicia_Timer0();	// Inicia o Timer 0
	
	
	//Limpa o buffer de recepção SPI
	while (i<64){SPI_Data[i]=0;i++;}i=0;

	
	while(1)
	{
			if(update_lcd)
			{
				sprintf(mensagem_lcd, "nBytes=%d T=%d  ", nBytes_SPI, transmissoes);  //Formata mensagem no buffer de dados
				mostra_lcd_buff(1,1, mensagem_lcd,16);
	
				sprintf(mensagem_lcd, "0x%X,%X,%X,%X,%X       ", SPI_Data[0], SPI_Data[1], SPI_Data[2], SPI_Data[3], SPI_Data[4]);  //Formata mensagem no buffer de dados
				mostra_lcd_buff(2,1, mensagem_lcd,16);

				update_lcd=0;
			}

		
		//Fim do loop While
	}
	
	//Fim do loop Main
}


//No caso de precisarmos...
void Inicia_Timer0(void)
{
	T0CONbits.TMR0ON=0;			//Timer 0 desligado para configuração
	T0CONbits.T08BIT=0;			//Seleção entre modo 8(1) e 16 bits(0).
	T0CONbits.T0CS=0;			//Seleção entre clock interno(0) ou transição no pino(1) T0CKI (RA4)
	T0CONbits.T0SE=0;			//Seleção entre resposta à rampa de subida(0) ou descida(1) no pino T0CKI.
	T0CONbits.PSA=0;			//Pre-scaler selecionado(0) ou "bypass"(1), onde o clock é injetado diretamente no timer.
	T0CONbits.T0PS=0b011;		//Divisão do Pre-scaler. Vai de 1:2 (0b000) à 1:256 (0b111);
	T0CONbits.TMR0ON=1;			//Timer 0 ligado!
}

/*------------------------------GARBAGE------------------------------------------






*/