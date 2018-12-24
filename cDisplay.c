#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>

#define LCD_CS 1
#define LCD_WIDTH  480
#define LCD_HEIGHT 320

int resetDisplay();
int sendToLCD(char *data, int len);
void sendColour(unsigned char colour);
void setFrame(int x, int y, int w, int h);
void sendCommand(unsigned char cmd);
void sendData(unsigned char data);


int main()
{
	//Open connection to lcd
	if(wiringPiSetup() < 0){
		exit(-1);
	}
	else{
		pinMode(10, OUTPUT);
		digitalWrite(10, HIGH);
	}
	if(wiringPiSPISetup(LCD_CS, 32000000)<0){
        exit(-1);
	}
   
	unsigned char command[3];
	command[0] = 0x11;
	command[1] = 0x00;
	command[2] = 0x00;
	
	unsigned char data[3];
	data[0] = 0x15;
	data[1] = 0x00;
	data[2] = 0x00;
	
	//Prepare LCD
	resetDisplay();
		
	//Software Reset
	sendCommand(0x01);
	delay(200);
	
	//Exit sleep mode, wait 5ms before seconding next command, load all default values
	sendCommand(0x11);
	delay(200);

	//RGB Interface Singal Control but did not select RBG interface
	sendCommand(0xB0);
	sendData(0x80);

	//Set backlight control but send too many, trying with 1 parameter (nothing changed)
	sendCommand(0xB9);
	sendData(0xFF);

	//Power control but send too many
	sendCommand(0xC0);
	sendData(0x13);

	//Step up circuit control, no idea what it is
	sendCommand(0xC1);
	sendData(0x08);
	sendData(0x0F);
	sendData(0x08);
	sendData(0x08);

	//Dont even have C4 for 9341 so assume it is 9340 which has it which is power for partial mode
	sendCommand(0xC4);
	sendData(0x11);
	sendData(0x07);
	sendData(0x03);
	sendData(0x04);

	//Dont even have C6 for 9340 so assume it is 9486 which has it for CABC Control
	sendCommand(0xC6);
	sendData(0x00);

	//CABC Control, what is that??? Again, too many parameters
	sendCommand(0xC8);
	sendData(0x03);
	sendData(0x03);
	sendData(0x13);
	sendData(0x5C);
	sendData(0x03);
	sendData(0x07);
	sendData(0x14);
	sendData(0x08);
	sendData(0x00);
	sendData(0x21);
	sendData(0x08);
	sendData(0x14);
	sendData(0x07);
	sendData(0x53);
	sendData(0x0C);
	sendData(0x13);
	sendData(0x03);
	sendData(0x03);
	sendData(0x21);
	sendData(0x00);

	//Control tearing Effect Line, current set to 0, try 1 later
	sendCommand(0x35);
	sendData(0x00);

	//Memory access control, currently set to 0110 0000 which sets MH to 1 thats all which means should be from right to left, to check vertical (no results, no matter what I put the way it refreshes doesnt change)
	//If 0x00 is sent, the screen will fill 3/4 only, 
	//Set to 60 to activate column address order(top down instead of bottom up) and row/column exchange(fill up whole screen instead of 3/4)
	sendCommand(0x36);
	sendData(0x60);

	//Set pixel format, current set to 0101(DPI set to 16bits) 0101(CPU interface set to 16 bit as well) dont need to test as unable to use 18 bit
	//Default is 06h which is 18bit
	sendCommand(0x3A);
	sendData(0x55);

	//Adjusts tearing
	sendCommand(0x44);
	sendData(0x00);
	sendData(0x01);

	//NV Memory write, again too many parameters, both are required, random value also can output
	sendCommand(0xD0);
	sendData(0x07);
	sendData(0x07);
	sendData(0x1D);
	sendData(0x03);

	//On the display
	sendCommand(0x29);
	
	printf("Making it black\n");
	setFrame(0, 0, 480, 320);
	
	sendColour(0x0F);
	for(int a = 0; a<480*320; a++)
	{
		digitalWrite(10, HIGH);
		digitalWrite(10, LOW);
	}

	FILE *f = fopen("//dev//fb0", "rb");
	
	return 0;
	
}

int sendToLCD(char *data, int len){
	int ret = 0;
    digitalWrite(10, HIGH);
	ret = wiringPiSPIDataRW(LCD_CS, (unsigned char*)data, len);
    digitalWrite(10, LOW);
    return ret;
}

void sendCommand(unsigned char cmd){
    unsigned char b1[3];
    b1[0] = 0x11;
	b1[1] = 0x00;
	b1[2] = cmd;
	sendToLCD(&b1[0], sizeof(b1));
}

void sendData(unsigned char data){
    unsigned char b1[3];
    b1[0] = 0x15;
	b1[1] = 0x00;
	b1[2] = data;
	sendToLCD(&b1[0], sizeof(b1));
}

int resetDisplay(){
	unsigned char toSend[4];
	toSend[0] = 0x00;
	toSend[1] = 0x01;
	toSend[2] = 0x00;
	toSend[3] = 0x00;
	
	sendToLCD(&toSend[0], sizeof(toSend));
	delay(100);
	
	toSend[1] = 0x00;
	sendToLCD(&toSend[0], sizeof(toSend));
	delay(100);
	
	toSend[1] = 0x01;
	sendToLCD(&toSend[0], sizeof(toSend));
	delay(100);
}

void sendColour(unsigned char colour){
	unsigned char toSend[3];
	toSend[0] = 0x15;
	toSend[1] = colour;
	toSend[2] = colour;
	sendToLCD(&toSend[0], sizeof(toSend));
}

void setFrame(int x, int y, int w, int h){
	sendCommand(0x2A);
	sendData(x>>8);
	sendData(x&0xFF);
	sendData(((w+x)-1)>>8);
	sendData(((w+x)-1)&0xFF);
	
	sendCommand(0x2B);
	sendData(y>>8);
	sendData(y&0xFF);
	sendData(((h+y)-1)>>8);
	sendData(((h+y)-1)&0xFF);

	sendCommand(0x2C);
}
	