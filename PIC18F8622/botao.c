#include <p18f8622.h>
#include <delays.h>
#include "botao.h"
#define horizontal PORTE
#define vertical PORTF
#define SET_VERTICAL TRISF=0x00
#define SET_HORIZONTAL TRISE=0xff
extern char s[32];
unsigned char var;
char search() {
int btn=0;
SET_HORIZONTAL;
switch(horizontal){
case 0x01: btn=1;
break;
case 0x02: btn=2;
break;
case 0x04: btn=3;
break;
case 0x08: btn=4;
break;
case 0x10: btn=5;
break;
case 0x20: btn=6;
break;
case 0x40: btn=7;
break;
case 0x80: btn=8; // verifica coluna
break;
}
if (btn) {
switch(var){
case 0x01: btn+=0;
break;
case 0x02: btn+=8;
break;
case 0x04: btn+=16;
break;
case 0x08: btn+=24;
break;
case 0x10: btn+=32;
break;
case 0x20: btn+=40;
break;
case 0x40: btn+=48;
break;
case 0x80: btn+=56; // verifica linha
break;
}
}
return btn;
}

char get(){
int btn=0;
SET_VERTICAL;
for (var=1;var; var<<=1)
{
vertical=var;
if (search()) btn=search();
}
return btn;
}
