#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/mman.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>
#include <stdint.h>

#define LCD_CS 1
#define LCD_WIDTH  480
#define LCD_HEIGHT 320
#define DELAY 400
#define	BLOCK_SIZE		(4*1024)
#define HIGH 1
#define LOW 0

int resetDisplay();
int sendToLCD(char *data, int len);
void sendColour(unsigned char colour);
void setFrame(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY);
void sendCommand(unsigned char cmd);
void sendData(unsigned char data);
void sendPixel(unsigned char pixel1, unsigned char pixel2);
void duplicateFrameBuffer(unsigned char *prevFrame, unsigned char *currentFrame);

static volatile unsigned int *gpio;

void digitalWrite(int a, int b){
	if(b){
		*(gpio + 7) = 1 << (8 & 31) ;
	}
	else{
		*(gpio + 10) = 1 << (8 & 31) ;
	}
}

int gpioSetup(){
	int fd = open("/dev/gpiomem", O_RDWR | O_SYNC | O_CLOEXEC);
	if(fd < 0){	// We're using gpiomem
		printf("Unable to open /dev/mem or /dev/gpiomem: %s.\n Aborting your program because if it can not access the GPIO hardware then it most certianly won't work\n", strerror(errno)) ;
		return -1; 
	}
	gpio = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x00200000) ;
	if (gpio == MAP_FAILED)
	{
		printf("mmap (GPIO) failed: %s\n", strerror (errno));
		return -1;
	}
	return 1;
}

int main()
{
	//Change framebuffer display size
	system("fbset -g 480 320 480 320 16");	
	//Open connection to lcd
	if(gpioSetup() < 0){
		exit(-1);
	}
	else{
		*(gpio + 0) = (*(gpio + 0) & ~(7 << 24)) | (1 << 24);
		//Set to high
		*(gpio + 7) = 1 << (8 & 31) ;
	}
	printf("Problem/n");
	if(wiringPiSPISetup(LCD_CS, 32000000)<0){
		printf("Problem/n");
        exit(-1);
	}

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
	
	//printf("Making it black\n");
	setFrame(0, 0, 479, 319);
	
	sendColour(0x99);
	for(int a = 0; a<480*320; a++)
	{
		digitalWrite(10, HIGH);
		digitalWrite(10, LOW);
	}
	
	
	// allocate memory for entire content
	unsigned char *prevFrame;
	prevFrame = calloc(480*320*2, 1);
	
	unsigned char *currentFrame;
	currentFrame = calloc(480*320*2, 1);
	if(!prevFrame ){
		printf("Memory Alloc for either prevFrame or currentFrame Fails\n");
		exit(1);
	}

	//Load first frame
	duplicateFrameBuffer(prevFrame,currentFrame);

	
	int pixelLocation;
	int topLeftX;
	int topLeftY;
	int lowerRightX;
	int lowerRightY;
	int readAmount;
	while(1){
		
		topLeftX = 480;
		topLeftY = 320;
		lowerRightX = 0;
		lowerRightY = 0;
		
		FILE *f;
		f = fopen("//dev//shm//Xvfb_screen0", "rb");
		fseek(f , -480*320*2, SEEK_END);
		if(fread(currentFrame , 1, 480*320*2, f) != 480*320*2){
			fclose(f);
			free(currentFrame);
			printf("Read framebuffer fails\n");
			exit(1);
		}
		fclose(f);

		//Find box to draw and convert framebuffer data				
		for(int y = 0; y<320; y++){
			for(int x = 0; x<480; x++){
				pixelLocation = ((y*480)+x)*2;
				
				if(prevFrame[pixelLocation] != currentFrame[pixelLocation] || prevFrame[pixelLocation+1] != currentFrame[pixelLocation+1]){				
					if(x < topLeftX){
						topLeftX = x;
					}
					if(y < topLeftY){
						topLeftY = y;
					}
					if(x > lowerRightX){
						lowerRightX = x;	
					}
					if(y > lowerRightY){
						lowerRightY = y;
						//printf("Update Y to %d\n", y); 	
					}
				}
				prevFrame[pixelLocation] = currentFrame[pixelLocation];
				prevFrame[pixelLocation+1] = currentFrame[pixelLocation+1];
			}
			//printf("Y: %d\n", y);
		}

		if(lowerRightX-topLeftX < 0){
			continue;
		}
		
		
		
		//printf("TopLeftX: %d, TopLeftY: %d, LowerRightX: %d, LowerRightY: %d\n", topLeftX, topLeftY, lowerRightX, lowerRightY);
		setFrame(topLeftX, topLeftY, lowerRightX, lowerRightY);
		
		int firstPixelLocation = topLeftY*480+topLeftX;
		
		unsigned char pixelA = currentFrame[firstPixelLocation+1];
		unsigned char pixelB = currentFrame[firstPixelLocation];
		unsigned char prevPixelA = currentFrame[firstPixelLocation+1];
		unsigned char prevPixelB = currentFrame[firstPixelLocation];
		sendPixel(pixelA, pixelB);
		
		for(int y = topLeftY; y<lowerRightY+1; y++)
		{
			for(int x = topLeftX; x<lowerRightX+1; x++)
			{
				if(y == topLeftY && x == topLeftX){
					continue;
				}
				
				pixelLocation = ((y*480)+x)*2;
				
				pixelA = currentFrame[pixelLocation+1];
				pixelB = currentFrame[pixelLocation];
				
				if(pixelA == prevPixelA && pixelB == prevPixelB)
				{
					digitalWrite(10, HIGH);
					digitalWrite(10, LOW);
					for(int b = 0; b<DELAY; b++){
					}
				}
				else
				{
					sendPixel(pixelA, pixelB);
				}
				prevPixelA = pixelA;
				prevPixelB = pixelB;
			}
		}
	}
	
	free(prevFrame);
	free(currentFrame);	
	printf("Done\n");
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

void sendPixel(unsigned char pixel1, unsigned char pixel2){
	unsigned char toSend[3];
	toSend[0] = 0x15;
	toSend[1] = pixel1;
	toSend[2] = pixel2;
	sendToLCD(&toSend[0], sizeof(toSend));
}

void setFrame(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY){
	sendCommand(0x2A);
	sendData(topLeftX>>8);
	sendData(topLeftX&0xFF);
	sendData((bottomRightX)>>8);
	sendData((bottomRightX)&0xFF);
	
	sendCommand(0x2B);
	sendData(topLeftY>>8);
	sendData(topLeftY&0xFF);
	sendData((bottomRightY)>>8);
	sendData((bottomRightY)&0xFF);

	sendCommand(0x2C);
}

void duplicateFrameBuffer(unsigned char *prevFrame, unsigned char *currentFrame){		
	FILE *f;
	f = fopen("//dev//shm//Xvfb_screen0", "rb");
	
	fseek(f , -480*320*2, SEEK_END);	
	if(fread(currentFrame , 1, 480*320*2, f) != 480*320*2){
		fclose(f);
		free(currentFrame);
		printf("Read framebuffer fails\n");
		exit(1);
	}
	fclose(f);
	
	//Draw frame
	setFrame(0,0, 479, 319);
	unsigned char pixelA = currentFrame[1];
	unsigned char pixelB = currentFrame[0];
	unsigned char prevPixelA = currentFrame[1];
	unsigned char prevPixelB = currentFrame[0];
	prevFrame[1] = pixelA;	
	prevFrame[0] = pixelB;
	
	sendPixel(pixelA, pixelB);
	
	for(int a =1; a<480*320+1; a++){
		pixelA = currentFrame[(a*2)+1];
		pixelB = currentFrame[a*2];
		
		if(pixelA == prevPixelA && pixelB == prevPixelB)
		{
			digitalWrite(10, HIGH);
			digitalWrite(10, LOW);
			for(int b = 0; b<DELAY; b++){
			}
		}
		else
		{
			sendPixel(pixelA, pixelB);
		}
		prevFrame[(a*2)+1] = pixelA;	
		prevFrame[(a*2)] = pixelB;
		prevPixelA = pixelA;
		prevPixelB = pixelB;
	}
}
