// DEFINES ==========================================================
#define LCD_DATA  PORTD            // Dataport of LCD-Display (D4..D7) 
#define LCD_RS    PORTDbits.RD1    // Register Select of LCD-Display
#define LCD_E     PORTDbits.RD2    // Enable of LCD-Display
#define CTRL_REG  0                // Select instruction register
#define DATA_REG  1                // Select data register
#define BLINK     0x01             // Alias for blinking cursor
#define NOBLINK   0x00             // Alias for non blinking cursor
#define SHOW      0x02             // Alias for cursor on
#define HIDE      0x00             // Alias for cursor off
#define ON        0x04             // Alias for display on
#define OFF       0x00             // Alias for display off
                                                              
// PROTOTYPES========================================================
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
