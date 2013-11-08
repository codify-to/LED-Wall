LED-Wall
========

## Naming the PICs
*Master*: PIC18F4550
*Slave*: PIC18F8622

## Downloads

* MPLAB® XC32 (Free): [http://www.microchip.com/pagehandler/en_us/devtools/mplabxc/]()
* MPLAB® X IDE: [http://www.microchip.com/pagehandler/en-us/family/mplabx/]()


## Master-slave protocol

byte0: size
byte1: board,x,y,color
byte2: led intensity


## Reading the serial messages:

    screen /dev/tty.usbmodem* 9600

#### To quit screen: `ctrl-a k`