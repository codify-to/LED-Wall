//----------------------------- Módulo para Displays LCD --------------------------------//

#include<delays.h> //Biblioteca necessária para gerar o delay

//----------------------------- Configuração de Hardware -------------------------------//

//Pinos do LCD

#define Fr_Mhz 8 //Velocidade do Microcontrolador: 48, 40, 20, 16, 12 e 8 Mhz. 

#define DB7 PORTDbits.RD7 	//Pino de dado 7 do LCD
#define DB6 PORTDbits.RD6 	//Pino de dado 6 do LCD
#define DB5 PORTDbits.RD5 	//Pino de dado 5 do LCD
#define DB4 PORTDbits.RD4 	//Pino de dado 4 do LCD
#define E  PORTDbits.RD3  	//Pino Enable do LCD
#define RS PORTDbits.RD2 	//Pino Register Select do LCD

void lcd_start();
void limpa_tela ();
void mostra_lcd_string(char line, char column, rom char *p_char);
void mostra_lcd_buff(char line, char column, char *p_char, char buffer_size);


void tempo_us(int tempo)// Gera 1*X us de delay.
{
	if(Fr_Mhz==48)
	{
		while(tempo>0)
		{
			Delay10TCY (); 	//Delay de 10 ciclos
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			tempo--;
		}
	}

	if(Fr_Mhz==40)
	{
		while(tempo>0)
		{
			Delay10TCY (); 	//Delay de 10 ciclos
			tempo--;
		}
	}

	if(Fr_Mhz==20)
	{
		while(tempo>0)
		{
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			tempo--;
		}
	}

	if(Fr_Mhz==16)
	{
		while(tempo>0)
		{
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo.
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			tempo--;
		}
	}

	if(Fr_Mhz==12)
	{
		while(tempo>0)
		{
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			tempo--;
		}
	}

	if(Fr_Mhz==8)
	{
		while(tempo>0)
		{
			Delay1TCY (); 	//Delay de 1 ciclo
			Delay1TCY (); 	//Delay de 1 ciclo
			tempo--;
		}
	}

}

void envia_lcd (char byte, char cmd_data, int delay)
{
	RS = cmd_data; //Informa se é um comando ou dado.

    DB7 = byte>>7;	//Envia bit 7 
    DB6 = byte>>6;	//Envia bit 6 
    DB5 = byte>>5;	//Envia bit 5 
    DB4 = byte>>4;	//Envia bit 4 
                
    E=1; tempo_us(2);E=0; //Habilta, espera 2uS, desabilita LCD
                
    DB7 = byte>>3;	//Envia bit 3 
    DB6 = byte>>2;	//Envia bit 2
    DB5 = byte>>1;	//Envia bit 1 
    DB4 = byte;		//Envia bit 0 
                        
    E=1; tempo_us(2);E=0; //Habilta, espera 2uS, desabilita LCD
                
    tempo_us(delay); //Delay necessário para comando enviado
}

//Cursor L1, C1
void volta_cursor ( )
{
        envia_lcd (0x02,0,2000);
}

//Cursor L1, C1 e limpa a tela.
void limpa_tela ( )
{
        envia_lcd (0x01,0,2000);
}

//Valor 0 move para direita. Valor 1 move para esquerda
void move_cursor(char lado)
{
	char cursor[2]={0x14, 0x10}; 
    envia_lcd (cursor[lado],0,40);
}

//Valor 0 move para direita. Valor 1 move para esquerda
void move_mensagem(char lado)
{
	char cursor[2] = {0x1c, 0x18};
    envia_lcd (cursor[lado],0,40);
}

//Controle do cursor e display
//Para cursor desligado = 0
//Para display desligado = 1
//Para cursor ligado piscando = 2
//Para display e cursor ligado = 3
//Para display ligado e cursor piscando = 4
void ctrl_lcd_cursor (char controle)
{
	char comando [5] = {0x0c, 0x08, 0x0f, 0x0e, 0x0d}; 
    envia_lcd (comando[controle],0,40);
}

//Coloca o cursor em uma determinada posição do LCD.
void pos_lcd(char line, char column)
{
	if(line==1)
	envia_lcd ((0x80+column-1),0,40);
	if(line==2)
	envia_lcd ((0xc0+column-1),0,40);
	if(line==3)
	envia_lcd ((0x94+column-1),0,40);
	if(line==4)
	envia_lcd ((0xd4+column-1),0,40); 
}

//Escreve um caractere ou símbolo no display.
void envia_dado(char dado)
{
        envia_lcd (dado,1,45);
}

//Mostra diretamente no LCD a string desejada
void mostra_lcd_string(char line, char column, rom char *p_char)
{
	pos_lcd(line, column);

    while (*p_char!=0)
    {
        envia_dado(*p_char);
        p_char++;
    }
}

//Mostra no LCD um buffer de dados contendo uma string
void mostra_lcd_buff(char line, char column, char *p_char, char buffer_size)
{
	pos_lcd(line, column);

	while (buffer_size--)
	{
    	envia_dado(*p_char);
    	p_char++;
	}
}

//Inicializa o LCD. Configuração padrão:
//lcd_start(0x28, 0x0F, 0x06);
//4 vias (DB7 ~ DB4)
//Duas linhas (16x2, 20x2, etc...)
//Matriz 8x5
//Liga display com cursor piscando e move ele para direita ao inserir caracteres

void lcd_start()
{
	char lcd_conf[6] = {0x03, 0x02, 0x00, 0x00, 0x00};
	char contagem=0;

	lcd_conf[2] = 0x28;
	lcd_conf[3] = 0x0F;
	lcd_conf[4] = 0x06;

	RS = 0; 	//Comando
	E = 0; 		//Desabilita lcd

    tempo_us(20000); //Gera um atraso de 20ms. 

    while(contagem<3)
    {
    	DB7 = lcd_conf[0]>>3;	//Envia bit 3
    	DB6 = lcd_conf[0]>>2;	//Envia bit 2
        DB5 = lcd_conf[0]>>1;	//Envia bit 1
        DB4 = lcd_conf[0];		//Envia bit 0 
                
    	E=1; tempo_us(2);E=0; //Habilta, espera 2uS, desabilita LCD
                
        tempo_us(5000); //Gera um delay de 5ms.
		contagem++;
    }

	DB7 = lcd_conf[1]>>3;	//Envia bit 3
	DB6 = lcd_conf[1]>>2;	//Envia bit 2
	DB5 = lcd_conf[1]>>1;	//Envia bit 1
	DB4 = lcd_conf[1];		//Envia bit 0 
        
    E=1; tempo_us(2);E=0; //Habilta, espera 2uS, desabilita LCD

	tempo_us(40); //Gera um delay de 40us.

	envia_lcd (lcd_conf[2],0,40);
	envia_lcd (lcd_conf[3],0,40);
	envia_lcd (lcd_conf[4],0,40);

	limpa_tela ( );
	ctrl_lcd_cursor (0);
}
