#include <p18f4431.h>
#include <delays.h>
#include <timers.h>
#include <pwm.h>
#include <stdio.h>       // Definição do Stdio para poder usar sprintf
#include "lcd.h"        // Definição das funções do lcd

/* Relação das IOs

RC3 Liga/Desliga
RC4 Primeiro grupo de chaves
RC5 Segundo grupo de chaves
RC6 Avnça Encoder
RC7 Retrocede Encoder
RC0-RC1 TIMER1 XTAL
RC2 PWM
RA0-RA2 Primeiro grupo de chaves
RA3-RA5 Segundo grupo de chaves
RB0-RB4 Encoders
RE0 Amp
RD2 LCD E
RD3 LCD R/S
RD0 LCD POWER



/*mudar isto quando migrar o software para o pic certo*/
#define SWICH PORTC
#define on PORTCbits.RC3
#define CH1 PORTCbits.RC4
#define CH2 PORTCbits.RC5
#define PREV PORTCbits.RC7
#define NEXT PORTCbits.RC6
#define LCD PORTBbits.RB5
#define AMP1 PORTBbits.RB6
#define AMP2 PORTEbits.RE0
#define MIC1  PORTAbits.RA0
#define LOWIMP1  PORTAbits.RA1
#define HIIMP1  PORTAbits.RA2
#define MIC2  PORTEbits.RE0
#define LOWIMP2  PORTEbits.RE1
#define HIIMP2 PORTEbits.RE2

// globals
int PWM=0x00;
char ligando=0x00;
char desligando=0x00;
char blinkprescaler;
int buffer=0;
void InterruptHandlerHigh (void);
char s[32]; //para escrever no lcd
char horas = 00; 
char minutos = 00;
char segundos = 0;
char lcd_change;
char time_change;
char ajusta_hora=0;
char pisco=0;
char ligado=0;
char sleep=0;
struct encoders {
	char name[20];
	char value;
	} rom ENCODER[15]= {		//para centralizar os nomes, edite-os, acrescentando espaços no começo
	{"01 -  Hall 1", 9},
	{"02 -  Hall 2", 13},
	{"03 -  Sala 1", 5},
	{"04 -  Sala 2", 1},
	{"05 -  Sala 3", 0},
	{"06 - Plate 1", 4},		
	{"07 - Plate 2", 6},
	{"08 - Plate 3", 2},
	{"09 - Delay 1", 15},
	{"10 - Delay 2", 11},
	{"11 - Chorus", 3},
	{"12 - Chorus Sala", 10},
	{"13 - Auto Wah", 14},
	{"14 - Flanger", 7},
	{"15 - Vibratto", 8}
};

struct chaveamento {
	char pos;
	char key[3];
} key1 = {0,{1,2,4}},   key2 = {0,{8,16,32}};
char blink=0;
void main (void){
signed char actual_encoder = 0;	
/*inicialização dos ports*/
TMR1H=0x80;
TRISC=0x00;
TRISD=0x00;
PORTD=0x01;
PORTC=0x00;
TRISC=0xfb;
TRISB=0x00;
PORTB=0x09;
TRISC=0x00;
PORTC=0x00;
TRISC=0xfb;
TRISA=0x00;
TRISE=0x00;
PORTE=0x00;
PORTA=0x00;
PORTCbits.RC2=0;

INTCON = 0x20;                //disable global and enable TMR0 interrupt
INTCON2 = 0x84;               //TMR0 high priority
RCONbits.IPEN = 1;            //enable priority levels
TMR0H = 0;                    //clear timer
TMR0L = 0;                    //clear timer
T0CON = 0x80;                 //set up timer0 - prescaler 1:8
INTCONbits.GIEH = 1;          //enable interrupts
TRISB = 0;					  //enable timer1 interrupt
TMR1H=0x80;
TMR1L=0x00;
T1CON=0x0f;
/*fim da inicialização dos ports*/

/*inicialização do lcd*/
Delay10KTCYx(30);
init_LCD();
sprintf (s, "                        "); // limpa o que já tem no visor
LCD_printxy(1,2,s);
control_LCD(ON,NOBLINK,HIDE);
sprintf (s, "LiveClass  %02d:%02d:%02d", horas, minutos, segundos);
LCD_printxy(1,1,s);
sprintf (s, "Espere");
LCD_printxy(1,2,s);
/*fim da inicialização do lcd*/
PORTA=0x00;
PIE1bits.TMR1IE=1;
LVDCON=0x3D;				  //enable low voltage detection
PIE2bits.LVDIE=1;			  //enable low voltage interrupt
Delay10KTCYx(255);
sprintf (s, "                        ");
LCD_printxy(1,2,s);
/*fim da inicialização do lcd*/
/*inicialização do PWM*/
OpenTimer2( TIMER_INT_OFF &
T2_PS_1_1 &
T2_POST_1_16 );
OpenPWM1(0xff);
SetDCPWM1(0);
/*fim da inicialização do PWM*/
while(1) { //loop de eventos
if (on) {			// botão de liga/desliga
	while(on) {}		//espera largar o botão para não dar efeito PWM
	if (!ligado) {	//se tiver desligado, liga
		PORTE|=1;
		ligado=0x01;
		actual_encoder=0;
	    lcd_change=1;
		PORTA=key1.key[key1.pos]+key2.key[key2.pos];
		desligando=0;
		ligando=1;
		ajusta_hora=0;
		} 
	else {			//de estiver ligado, desliga
		PORTE&=0xfe;
		ligado = 0x00;
		sprintf (s, "                        "); // limpa o que já tem no visor
		LCD_printxy(1,2,s);
		PORTA=0;
		blink=0;
		key1.pos=key2.pos=0;
		ligando=0;
		desligando=1;
		ajusta_hora=0;
        }
	Delay10KTCYx(5);
}

if (ligado){


	if (CH1) {

		if (blink&0x01) {		//verifica se estamos no modo piscante
		int timer=0;
			while(CH1) {	// se tiver, cronometra o tempo que o botão CH1 está
				Delay10KTCYx(1);// sendo apertado
				timer++;
			}
			if (timer>200) {     //se for maior que 100 estamos com hold, o 200 pode ser alterado
				blink&=0xFE;         //sai do modo piscante
				if(!blink) PORTE|=1; //liga o amplificador do canal 1
		        PORTA=key1.key[key1.pos]+key2.key[key2.pos];
		    	}
			else {				 //do contrário basta mudar a entrada 	
			key1.pos++;
			if (key1.pos > 2) key1.pos=0;
			PORTA=key1.key[key1.pos]+key2.key[key2.pos];
				}
		}
		else { // do contrário entra no modo piscante
			while(CH1);
			blink |= 0x01;
			PORTE&=0xfe; // e desliga o amplificador
		}
		//sprintf (s, "%02d  %02d  %2d                   ", PORTA, key1.key[key1.pos]+key2.key[key2.pos],key1.pos);
	//	LCD_printxy(1,1,s);
	}


	if (CH2) {

		if (blink&0x02) {		//verifica se estamos no modo piscante
		int timer=0;
			while(CH2) {	// se tiver, cronometra o tempo que o botão CH1 está
				Delay10KTCYx(1);// sendo apertado
				timer++;
			}
			if (timer>200) {     //se for maior que 100 estamos com hold, o 200 pode ser alterado
				blink&=0xFD;         //sai do modo piscante
				if(!blink) PORTE|=1; //liga o amplificador do canal 2
			    PORTA=key1.key[key1.pos]+key2.key[key2.pos];
	    		}
			else {				 //do contrário basta mudar a entrada 
			key2.pos++;
			if (key2.pos > 2) key2.pos=0;
			PORTA=key1.key[key1.pos]+key2.key[key2.pos];
				}
		}
		else { // do contrário entra no modo piscante
			while(CH2);
			blink |= 0x02;
			PORTE&=0xfe; // e desliga o amplificador
		}
	//	sprintf (s, "%02d  %02d  %2d                    ", PORTA, key1.key[key1.pos]+key2.key[key2.pos], key2.pos);
	//	LCD_printxy(1,1,s);
	}

	if (NEXT) {
		int timer=0;									// detecta o delay
		while (NEXT){
			Delay10KTCYx(1);
			timer++;
		}
		if (timer>200) ajusta_hora=(ajusta_hora^0x02)&0x02;				//se maior que 200 entra no modo deajuste de hora
		else {
			if (ajusta_hora&0x01) {						//se no modo de ajuste de hora, incrementa a hora
				horas++;
				if (horas==24) horas=0;
				segundos=0;  
			}
			if (ajusta_hora&0x02) {						//se no modo de ajuste de hora, incrementa a hora
				minutos++;
				if (minutos==60) minutos=0;
				segundos=0;  
			}
			if (!ajusta_hora){
	    		actual_encoder++;                              //do contrário próximo efeito
				if (actual_encoder==0x0f) actual_encoder=0x00;
				lcd_change=1;
			}
		}
   	}
	if (PREV) {
		int timer=0;										//detecta o delay
		while (PREV){
			Delay10KTCYx(1);
			timer++;
		}
		if (timer>200) ajusta_hora=(ajusta_hora^0x01)&0x01;					//se maior que 200 entra no ajuste de minuto
		else {
			if (ajusta_hora&0x01) {						//se no modo de ajuste de hora, incrementa a hora
				horas--;
				if (horas==-1) horas=23;
				segundos=0;  
			}
			if (ajusta_hora&0x02) {							//se em ajuste de minuto, incrementa o minuto
				minutos--;
				if (minutos==-1) minutos=59;
				segundos=0; 
			}
			if (!ajusta_hora){
	    		actual_encoder--;                              // efeito anterior
				if (actual_encoder==0xffff) actual_encoder=0x0e;
				lcd_change=1;
			}
		}
   	}
	SWICH = (SWICH&0xF0)|(ENCODER[actual_encoder].value&0x0F); //insere o efeito no port junto com as chaves

}
	if (lcd_change) {
		sprintf (s, "                    "); // limpa o que já tem no visor
		LCD_printxy(1,2,s);
		LCD_printxy_rom(1,2,ENCODER[actual_encoder].name);// escreve o encoder ativo
		if (!ligado) LCD_printxy(1,2,s);// se estiver desligado, apaga a linha de baixo
		PORTB=(PORTB&0xF0)|(ENCODER[actual_encoder].value&0x0F);
		control_LCD(ON,NOBLINK,HIDE);
		Delay10KTCYx(30);
		lcd_change=0;
		}
	if (time_change) {
		sprintf (s, "LiveClass  %02d:%02d:%02d", horas, minutos, segundos);//atualiza a hora
		LCD_printxy(1,1,s);
		Delay10KTCYx(30);
		time_change=0;
	}
	if (pisco&0x02) {
		pisco&=0xfD;
		if (ajusta_hora&0x01) {	
			if(pisco&0x01) {		//apaga o vlor das horas
				sprintf (s, "  ");
				LCD_printxy(12,1,s);
			}
			else {
				sprintf (s, "%02d",horas); //escreve o valor das horas
				LCD_printxy(12,1,s);
				Delay10KTCYx(30);
			}
		}
		if (ajusta_hora&0x02) {
			if(pisco&0x01) {			//apaga o valor dos minutos
				sprintf (s, "  ");
				LCD_printxy(15,1,s);
			}
			else {
				sprintf (s, "%02d",minutos); //escreve o valor dos minutos
				LCD_printxy(15,1,s);
				Delay10KTCYx(30);
			}
		}
	}
}
}

//----------------------------------------------------------------------------
// High priority interrupt vector

#pragma code InterruptVectorHigh = 0x08
void
InterruptVectorHigh (void)
{
  _asm
    goto InterruptHandlerHigh //jump to interrupt routine
  _endasm
}

//----------------------------------------------------------------------------
// High priority interrupt routine

#pragma code
#pragma interrupt InterruptHandlerHigh


void
InterruptHandlerHigh ()
{
  if (INTCONbits.TMR0IF)
    {                                   //check for TMR0 overflow
	    TMR0L=0x00;					  // mude estes dois valores
	    TMR0H=0xF0;					  // para alterar o tempo de exibição
		blinkprescaler++;
		if (blinkprescaler==0x20) {
	      	if (blink&0x01) LATA ^= key1.key[key1.pos]; //toggle LED on RB0
    	  	if (blink&0x02) LATA ^= key2.key[key2.pos] ;//toggle LED on RB0
			pisco^=3;			//controla a velocidade que os valores de minutos e horas piscam 
			blinkprescaler=0x00; // quando no modo de ajuste
			}
      INTCONbits.TMR0IF = 0;            //clear interrupt flag
		if (ligando) {
			PWM++;
			SetDCPWM1(PWM);
			if (PWM==0x3ff) {		//efeito de pwm para ligar
				ligando=0;
			}
		}
		if (desligando){ 
			PWM--;
			SetDCPWM1(PWM);			//efeito de pwm para desligar
			if (PWM==0x00) { 
				desligando=0;
			}	
		}
	}
  if (PIR1bits.TMR1IF)

    {                                   //check for TMR1 overflow
      PIR1bits.TMR1IF = 0;            //clear interrupt flag
	  TMR1H=0x80;
	  segundos++;
	  time_change=1;				//relógio
      if (segundos == 60) {
      segundos = 0;
      minutos ++;
		if (minutos == 60) {
		minutos = 0;
		horas ++;
		 if (horas == 24) {
		 horas = 0;
		 }
        }
	  }
	 if (sleep) {
		if (!PIR2bits.LVDIF){						//reativa o visor depois do sleep
	        PORTD|=0x01;
			Delay10KTCYx(30);
			init_LCD();
			sprintf (s, "                        "); // limpa o que já tem no visor
			LCD_printxy(1,2,s);
			control_LCD(ON,NOBLINK,HIDE);
			sprintf (s, "LiveClass  %02d:%02d:%02d", horas, minutos, segundos);
			LCD_printxy(1,1,s);
			OpenTimer2( TIMER_INT_OFF &
			T2_PS_1_1 &
			T2_POST_1_16 );
			T0CON = 0x80;
			OpenPWM1(0xff);
			SetDCPWM1(0);
			sleep=0;
		
			}
	 	}
	PIE2bits.LVDIE=1;
	}
   if (PIR2bits.LVDIF){
	blink=0;
	ligado=0;
	ligando=0;
	desligando=0;			//sequencia para dormir
	PWM=0;	
	PORTB=0;
	PORTA=0;
	PORTD=0;
	PORTE=0;
	PIR2bits.LVDIF=0;
	PIE2bits.LVDIE=0;
	CloseTimer2();
	ClosePWM1();
	PORTC=0;
	T0CON = 0x00;
	OSCCON=0x81;
	sleep=1;
	ajusta_hora=0;
	key1.pos=0;
	key2.pos=0;
	Sleep();
	}
}
//----------------------------------------------------------------------------
