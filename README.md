LED-Wall
========

## Naming the PICs
*Master*: PIC18F4550
*Slave*: PIC18F8622

## Downloads
**Compiler**: http://www.microchip.com/pagehandler/en_us/devtools/mplabxc/
You'll need this one: MPLAB® XC8

**IDE**: http://www.microchip.com/pagehandler/en-us/family/mplabx/


## Master-slave protocol

byte0: size  
byte1: board,x,y,color  
byte2: led intensity  


## Reading the searial messages:
	
	screen /dev/tty.usbmodem* 9600
#### To quit screen: <code>ctrl-a k</code>