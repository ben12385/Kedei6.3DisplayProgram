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


#define IRQ_PIN 6
#define Touch_CS 0

int main()
{	
	//Open connection to lcd
	if(wiringPiSetup() < 0){
		exit(-1);
	}
	else{
		pinMode(IRQ_PIN, INPUT);
	}
	while(1){
		if(digitalRead(6) == 0){
			printf("Touch detected\n");
		}
	}
	if(wiringPiSPISetup(TOUCH_CS, 32000000)<0){
        exit(-1);
	}
	
	unsigned char data[1];
	
	
	ret = wiringPiSPIDataRW(TOUCH_CS, data, 1);
	
}