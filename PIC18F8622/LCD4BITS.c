


// INCLUDES =========================================================
#include <p18f8622.h>    // Definição dos ports, registradores, etc
#include<delays.h>      // Definição das funções de delay
// END INCLUDES =====================================================


// DEFINES ==========================================================
#define LCD_DATA  PORTD           // Dataport of LCD-Display (D4..D7) 
#define LCD_RS    PORTDbits.RD3    // Register Select of LCD-Display
#define LCD_E     PORTDbits.RD2    // Enable of LCD-Display
#define CTRL_REG  0                // Select instruction register
#define DATA_REG  1                // Select data register
#define BLINK     0x01             // Alias for blinking cursor
#define NOBLINK   0x00             // Alias for non blinking cursor
#define SHOW      0x02             // Alias for cursor on
#define HIDE      0x00             // Alias for cursor off
#define ON        0x04             // Alias for display on
#define OFF       0x00             // Alias for display off
                                                                                    
// END DEFINES ======================================================


// Prototypes =======================================================
void delay(unsigned int time_100us);    // Timer dependend delay-routine
void init_LCD(void);                    // Initialize the LCD display
void clrscr(void);                      // Clears LCD screen
void LCD_putchar(unsigned char value);  // Writes a character to display
// Prints a text to x/y position
void LCD_printxy(unsigned char x,unsigned char y, unsigned char *text);
// Controls the display
void control_LCD(unsigned char dsp,unsigned char blink,unsigned char cursor);
// Sets LCD write position
void gotoxy(unsigned char x,unsigned char y);

// END Prototypes ===================================================


// Globals ==========================================================

// Table to select DD-RAM address (including instruction and address)
// 0x00..0x0F -> Line 1, 0x40..0x4F -> Line 2
static unsigned char LOCATION[2][16] = { {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
                                               0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F},
                                              {0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
                                               0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF} };

volatile unsigned int  DELAY;                   // Global delaytime

// END Globals ======================================================


void delay(unsigned int time_100us)
{
Delay10TCYx(240);
}

/********************************************************************/
/* Function    : init_LCD()                                     SUB */
/*------------------------------------------------------------------*/
/* Description : Routine initializes the LCD display recommended    */
/*               in the datasheet (4 Bit initialization)            */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau,  Modified by Zephirus              */
/*------------------------------------------------------------------*/
/* Input       : none                                               */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*               09/99    V1.1 Timing correction of init            */
/*				 07/07    V2.0 Ported to PIC						*/
/*                                                                  */
/********************************************************************/
void init_LCD(void)
{
TRISD=0;
delay(200);             // Wait 20ms
LCD_E  = 0;
LCD_RS = CTRL_REG;      // Switch to inruction register

//Set LCD_DATA to high nibble of Software Reset
LCD_DATA = (LCD_DATA&0x0F)|0x30;
LCD_E = 1; LCD_E = 0;   // Write data to display
delay(50);              // Wait 5ms

LCD_E = 1; 
LCD_E = 0;   // Write data to display again (SW Reset)
delay(50);              // Wait 5ms

LCD_E = 1; 
LCD_E = 0;   // Write data to display again (SW Reset)
delay(50);              // Wait 5ms

// Set LCD_DATA to high nibble of Function Set (4Bit)
LCD_DATA = (LCD_DATA&0x0F)|0x20;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
delay(4);

// Set LCD_DATA to high nibble of Function Set : 4 bit, 2 lines, 5*7 font
LCD_DATA = (LCD_DATA&0x0F)|0x20;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
// Set LCD_DATA to lower nibble of Function Set : 4 bit, 2 lines, 5*7 font
LCD_DATA = (LCD_DATA&0x0F)|0x80;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
delay(4);               // Wait 400µs

// Set LCD_DATA to high nibble of Display On/Off Control : display off, cursor off, don´t blink
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
// Set LCD_DATA to lower nibble of Display On/Off Control : display off, cursor off, don´t blink
LCD_DATA = (LCD_DATA&0x0F)|0x80;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
delay(4);               // Wait 400µs

// Set LCD_DATA to high nibble of Clear Display
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
// Set LCD_DATA to lower nibble of Clear Display
LCD_DATA = (LCD_DATA&0x0F)|0x10;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
delay(50);              // Wait 5ms

// Set LCD_DATA to high nibble of Entry Mode Set : increment DD-RAM address, move cursor
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
// Set LCD_DATA to lower nibble of Entry Mode Set : increment DD-RAM address, move cursor
LCD_DATA = (LCD_DATA&0x0F)|0x60;
LCD_E = 1; 
LCD_E = 0;   // Write data to display
delay(4);               // Wait 400µs

return;
}


/********************************************************************/
/* Function    : control_LCD(dsp,blink,cursor)                  SUB */
/*------------------------------------------------------------------*/
/* Description : Routine controls the screen                        */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8, Modified by Zephirus          */
/*------------------------------------------------------------------*/
/* Input       : unsigned char dsp    = ON     -> Display on        */
/*                                      OFF    -> Display off       */
/*               unsigned char blink  = BLINK  -> Cursor blinks     */
/*                                      NOBLINK-> Cursor not blinks */
/*               unsigned char cursor = SHOW   -> Cursor visible    */
/*                                      HIDE   -> Cursor invisible  */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*               09/99    V1.1 Calculation of control modified      */
/*                             (because of compiler otim. bug)      */
/*				 07/07    V2.0 Ported to PIC						*/
/*                                                                  */
/********************************************************************/
void control_LCD(unsigned char dsp,unsigned char blink,unsigned char cursor)
{
unsigned char control;  // variable to generate instruction byte

control = (0x08 + blink + cursor + dsp); // Cursor control

LCD_RS = CTRL_REG;      // Switch to instruction register
// Set LCD_DATA to high nibble of Display On/Off Control
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; LCD_E = 0;   // Write data to display
// Set LCD_DATA to lower nibble of Display On/Off Control
LCD_DATA = (LCD_DATA&0x0F)|((control<<4)&0xF0);
LCD_E = 1; LCD_E = 0;   // Write data to display
                 // Enable all interrupts

delay(4);               // Wait 400µs

return;
}


/********************************************************************/
/* Function    : gotoxy(x,y)                                    SUB */
/*------------------------------------------------------------------*/
/* Description : Sets the write position of the LCD display         */
/*                                                                  */
/*                 (1,1)         (16,1)                             */
/*                   |              |                               */
/*                   ################   -> line 1                   */
/*                   ################   -> line 2                   */
/*                   |              |                               */
/*                 (1,2)         (16,2)                             */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8, Modified by Zephirus          */
/*------------------------------------------------------------------*/
/* Input       : unsigned char x    -> x position (horizontal)      */
/*               unsigned char y    -> y position (vertical)        */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*				 07/07    V2.0 Ported to PIC						*/
/*                                                                  */
/********************************************************************/
void gotoxy(unsigned char x,unsigned char y)
{


LCD_RS = CTRL_REG;              // Switch to instruction register
// Set LCD_DATA to high nibble of position table value
LCD_DATA = (LCD_DATA&0x0F)|((LOCATION[y-1][x-1])&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display
// Set LCD_DATA to lower nibble of position table value
LCD_DATA = (LCD_DATA&0x0F)|(((LOCATION[y-1][x-1])<<4)&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display


delay(4);                       // Wait 400µs

return;
}


/********************************************************************/
/* Function    : clrscr()                                       SUB */
/*------------------------------------------------------------------*/
/* Description : Clears LCD display, and sets position to (1,1)     */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8, modified by Zephirus          */
/*------------------------------------------------------------------*/
/* Input       : none                                               */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*               09/99    V1.1 Timing correction                    */
/*				 07/07    V2.0 Ported to PIC						*/
/*                                                                  */
/********************************************************************/
void clrscr(void)
{
 

LCD_RS = CTRL_REG;              // Switch to instruction register
// Set LCD_DATA to high nibble of Clear Screen
LCD_DATA = (LCD_DATA&0x0F)|0x00;
LCD_E = 1; LCD_E = 0;           // Write data to display
// Set LCD_DATA to lower nibble of Clear Screen
LCD_DATA = (LCD_DATA&0x0F)|0x10;
LCD_E = 1; LCD_E = 0;           // Write data to displays

delay(41);                      // Wait 4.1ms

return;
}


/********************************************************************/
/* Function    : LCD_putchar(value)                             SUB */
/*------------------------------------------------------------------*/
/* Description : Writes the character value to the display          */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8, Modified by Zephirus          */
/*------------------------------------------------------------------*/
/* Input       : unsigned char value    -> character to be written  */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*				 07/07    V2.0 Ported to PIC						*/
/*                                                                  */
/********************************************************************/
void LCD_putchar(unsigned char value)
{


LCD_RS = 1;              // Switch to data register
// Set LCD_DATA to high nibble of value
LCD_DATA = (LCD_DATA&0x0F)|(value&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display
// Set LCD_DATA to lower nibble of value
LCD_DATA = (LCD_DATA&0x0F)|((value<<4)&0xF0);
LCD_E = 1; LCD_E = 0;           // Write data to display

delay(4);                       // Wait 400µs

return;
}


/********************************************************************/
/* Function    : LCD_printxy(x,y,*text)                         SUB */
/*------------------------------------------------------------------*/
/* Description : Prints text to position x/y of the display         */
/*------------------------------------------------------------------*/
/* Author      : Thorsten Godau  NT8, Modified By Zephirus          */
/*------------------------------------------------------------------*/
/* Input       : unsigned char x     -> x position of the display   */
/*               unsigned char y     -> y position of the display   */
/*               unsigned char *text -> pointer to text             */
/*------------------------------------------------------------------*/
/* Returnvalue : none                                               */
/*------------------------------------------------------------------*/
/* History     : 06/99    V1.0 Basic routine                        */
/*				 07/07    V2.0 Ported to PIC						*/
/*                                                                  */
/********************************************************************/
void LCD_printxy(unsigned char x,unsigned char y, unsigned char *text)
{
gotoxy(x,y);            // Set cursor position

while( *text )          // while not end of text
  {
  LCD_putchar(*text++); // Write character and increment position
  } 

return;
}

void LCD_printxy_rom(unsigned char x,unsigned char y, unsigned rom char *text)
{
gotoxy(x,y);            // Set cursor position

while( *text )          // while not end of text
  {
  LCD_putchar(*text++); // Write character and increment position
  } 

return;
}




/* EXEMPLO DE USO
// MAIN =============================================================
void main( void )
{
init_MCU();                                            // Initialize MCU
init_LCD();                                            // Initialize the LCD display
                
LCD_printxy(1,1,"CÓDIGO DOIODO");                   // Message to LCD
LCD_printxy(1,2,"MAS FUNCIONA");                   // Message to LCD
control_LCD(ON,NOBLINK,HIDE);                          // Display on, cursor hidden and non-blinking

while(1) // Main loop
  {
  // Do nothing
  _asm
    nop
  _endasm;
  }
}
// END MAIN =========================================================
*/
