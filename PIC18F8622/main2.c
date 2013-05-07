#include <p18f8622.h>
#include <delays.h>
#include <usart.h>
#include<stdio.h>       // Definição do Stdio para poder usar sprintf
#include "lcd.h"        // Definição das funções do lcd

/*mudar isto quando migrar o software para o pic certo*/
#define SWICH PORTJ
#define CHAVE0 PORTJbits.RJ0
#define CHAVE1 PORTJbits.RJ1
#define CHAVE2 PORTJbits.RJ2
#define CHAVE3 PORTJbits.RJ3
#define EFF1 PORTJbits.RJ4
#define EFF2 PORTJbits.RJ5
#define EFF3 PORTJbits.RJ6
#define EFF4 PORTJbits.RJ7
#define KEYS PORTE
#define KEY1 PORTEbits.RE0
#define KEY2 PORTEbits.RE1
#define KEY3 PORTEbits.RE2
#define KEY4 PORTEbits.RE4
#define NEXT PORTEbits.RE5
#define PREV PORTEbits.RE6
#define HIGHPORT PORTF // eu deveria colocar os pinos no vcc, mas gato escaldado tem medo de agua fria

char s[32]; //para escrever no lcd

/*struct encoders 
{
char name[10];
char value;
} ENCODER[15]; //tabela de encoders
int actual_encoder = 0;
*/
void main (void){

/*inicialização dos valores dos encoders*/
/*
sprintf (ENCODER[0].name, "Hall 1");
ENCODER[0].value = 9;
sprintf (ENCODER[1].name, "Hall 2");
ENCODER[1].value = 13;
sprintf (ENCODER[2].name, "Sala 1");
ENCODER[2].value = 5;
sprintf (ENCODER[3].name, "Sala 2");
ENCODER[3].value = 1;
sprintf (ENCODER[4].name, "Sala 3");
ENCODER[4].value = 0;
sprintf (ENCODER[5].name, "Plate 1");
ENCODER[5].value = 4;
sprintf (ENCODER[6].name, "Plate 2");
ENCODER[6].value = 6;
sprintf (ENCODER[7].name, "Plate 3");
ENCODER[7].value = 2;
sprintf (ENCODER[8].name, "Delay 1");
ENCODER[8].value = 15;
sprintf (ENCODER[9].name, "Delay 2");
ENCODER[9].value = 11;
sprintf (ENCODER[10].name, "Chorus");
ENCODER[10].value = 3;
sprintf (ENCODER[11].name, "Chorus Sala");
ENCODER[11].value = 10;
sprintf (ENCODER[12].name, "Auto-Wah");
ENCODER[12].value = 14;
sprintf (ENCODER[13].name, "Flanger");
ENCODER[13].value = 7;
sprintf (ENCODER[14].name, "Vibrato");
ENCODER[14].value = 8;
/*fim da inicialização dos valores dos encoders*/
init_LCD();
sprintf (s, "LiveClass");
LCD_printxy(1,1,s);
control_LCD(ON,BLINK,SHOW);
while(1);
//LCD_printxy(1,2,ENCODER[actual_encoder].name);
}
