#include <p18f4431.h>
#include <delays.h>
#include <usart.h>
#include<stdio.h>       // Definição do Stdio para poder usar sprintf
#include "lcd.h"        // Definição das funções do lcd

/*mudar isto quando migrar o software para o pic certo*/
#define SWICH PORTC
#define on PORTCbits.RC6
#define CH1 PORTCbits.RC2
#define CH2 PORTCbits.RC3
#define PREV PORTCbits.RC5
#define NEXT PORTCbits.RC4
#define LCD PORTBbits.RB4
#define AMP1 PORTBbits.RB6
#define AMP2 PORTBbits.RB5
#define MIC1  PORTAbits.RA0
#define LOWIMP1  PORTAbits.RA1
#define HIIMP1  PORTAbits.RA2
#define MIC2  PORTEbits.RE0
#define LOWIMP2  PORTEbits.RE1
#define HIIMP2 PORTEbits.RE2

// globals

void InterruptHandlerHigh (void);
char s[32]; //para escrever no lcd
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
char ligado=0;
signed char actual_encoder = 0;	
/*inicialização dos ports*/
TRISC=0x00;
PORTC=0x00;
TRISC=0xff;
TRISB=0x00;
PORTB=0x09;
TRISC=0x00;
PORTC=0x00;
TRISC=0xff;
TRISA=0x00;
TRISE=0x00;
PORTA=0x00;

INTCON = 0x20;                //disable global and enable TMR0 interrupt
INTCON2 = 0x84;               //TMR0 high priority
RCONbits.IPEN = 1;            //enable priority levels
TMR0H = 0;                    //clear timer
TMR0L = 0;                    //clear timer
T0CON = 0x82;                 //set up timer0 - prescaler 1:8
INTCONbits.GIEH = 1;          //enable interrupts
TRISB = 0;

/*fim da inicialização dos ports*/
/*inicialização do lcd*/
Delay10KTCYx(30);
init_LCD();
sprintf (s, "                        "); // limpa o que já tem no visor
LCD_printxy(1,2,s);
control_LCD(ON,NOBLINK,HIDE);
sprintf (s, "LiveClass  00:00");
LCD_printxy(1,1,s);
sprintf (s, "   Aguarde...");
LCD_printxy(1,2,s);
/*fim da inicialização do lcd*/
PORTA=0x00;
Delay10KTCYx(255);
Delay10KTCYx(255);
Delay10KTCYx(255);
Delay10KTCYx(255);
Delay10KTCYx(255);
sprintf (s, "                        ");
LCD_printxy(1,2,s);
while(1) { //loop de eventos
char lcd_change;
if (on) {			// botão de liga/desliga
	if (!ligado) {	//se tiver desligado, liga
		AMP2=1;
		LCD=1;
		ligado=0x01;
		actual_encoder=0;
	    lcd_change=1;
		PORTA=key1.key[key1.pos]+key2.key[key2.pos];	
		} 
	else {			//de estiver ligado, desliga
		AMP2=0;
		LCD=0;
		ligado = 0x00;
		sprintf (s, "                        "); // limpa o que já tem no visor
		LCD_printxy(1,2,s);
		PORTA=0;
		blink=0;
		key1.pos=key2.pos=0;
        }
	while(on);		//espera largar o botã para não dar efeito PWM
}

if (ligado){

	if (CH1) {

		if (blink&0x01) {		//verifica se estamos no modo piscante
		int timer=0;
			while(CH1) {	// se tiver, cronometra o tempo que o botão CH1 está
				Delay10KTCYx(1);// sendo apertado
				timer++;
			}
			if (timer>200) {     //se for maior que 100 estamos com hold, o 100 pode ser alterado
				blink&=0xFE;         //sai do modo piscante
				if(!blink) AMP2=1; //liga o amplificador do canal 1
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
			AMP2=0; // e desliga o amplificador
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
			if (timer>200) {     //se for maior que 100 estamos com hold, o 100 pode ser alterado
				blink&=0xFD;         //sai do modo piscante
				if(!blink) AMP2=1; //liga o amplificador do canal 2
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
			AMP2=0; // e desliga o amplificador
		}
	//	sprintf (s, "%02d  %02d  %2d                    ", PORTA, key1.key[key1.pos]+key2.key[key2.pos], key2.pos);
	//	LCD_printxy(1,1,s);
	}

	if (NEXT) {
	    actual_encoder++;                              // próximo efeito
		if (actual_encoder==0x0f) actual_encoder=0x00;
		lcd_change=1;
		while(NEXT);	
   	}
	if (PREV) {
   	 actual_encoder--;                             // efeito anterior
   	 if (actual_encoder==0xffff) actual_encoder=0x0e;
		lcd_change=1;
		while(PREV);	
   	}
	SWICH = (SWICH&0xF0)|(ENCODER[actual_encoder].value&0x0F); //insere o efeito no port junto com as chaves

}

if (lcd_change) {
	sprintf (s, "                        "); // limpa o que já tem no visor
	LCD_printxy(1,2,s);
	LCD_printxy_rom(1,2,ENCODER[actual_encoder].name);
if (!ligado) LCD_printxy(1,2,s);
	PORTB=(PORTB&0xF0)|(ENCODER[actual_encoder].value&0x0F);
	control_LCD(ON,NOBLINK,HIDE);
	Delay10KTCYx(30);
	lcd_change=0;
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
      INTCONbits.TMR0IF = 0;            //clear interrupt flag
      if (blink&0x01) LATA ^= key1.key[key1.pos]; //toggle LED on RB0
      if (blink&0x02) LATA ^= key2.key[key2.pos] ;//toggle LED on RB0
    }
}

//----------------------------------------------------------------------------
