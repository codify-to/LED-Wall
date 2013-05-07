#include <usart.h>
#include <p18f8622.h>
#include "lcd.h"
extern char s[32];

int BaudDetect() {

	char c, end, err, count2, received;;
	err=0;
	c=255;
	count2=0;
	end=0;
	Close1USART(); // fechar a usart, só por garantia
	while (c<400 | end!=0){
		PORTJ=err;
		Open1USART(USART_TX_INT_OFF & 
				  USART_RX_INT_OFF  &
				  USART_ASYNCH_MODE &
			  	  USART_EIGHT_BIT   &
        		  USART_CONT_RX     &
       	    	  USART_BRGH_LOW   ,
          	 	  c);
		PORTJ = c;
		err=0;
		while (count2<64 & err==0){
			Write1USART(count2);
			while (Busy1USART()) PORTBbits.RB1=1;
			PORTBbits.RB1=0;
			while (!DataRdy1USART()) PORTAbits.RA0=1;
			PORTAbits.RA0=0;
			count2++;
			received=Read1USART();
			if (received==count2) count2++; //faz a contagem de 1 a 64 e verifica o retorno
			else {
				err=1;
				count2=0;
			}
		}
		if (err=0){ 
			end++;
			Write1USART(0xff); // avisa a outra parte para sair do modo de teste
							   // depois caso tenha sido transmitido o pacte inteiro com sucesso
		}

			c++;			// do contrario, decrementa o contador.
			Close1USART();		// e fecha a porta serial.
  	}
return c-1;
}
