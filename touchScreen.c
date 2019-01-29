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
#define TOUCH_CS 0

int main()
{	
	//Open connection to lcd
	if(wiringPiSetup() < 0){
		exit(-1);
	}
	else{
		pinMode(IRQ_PIN, INPUT);
	}
	if(wiringPiSPISetup(TOUCH_CS, 32000000)<0){
        exit(-1);
	}
	
	int preventRepeat = 0;
	int ret = 0;
	unsigned char data[3];
	data[1] = 0;
	data[2] = 0;
	
	while(1){
		preventRepeat = 1;
		printf("Touch detected1\n");
		data[0] = 0x93;			//10010011
		data[1] = 0;
		data[2] = 0;
	
		ret = wiringPiSPIDataRW(TOUCH_CS, data, 3);
		printf("%d\n", data[1]);
		printf("%d\n\n", data[2]);
		
		data[0] = 0xD3;			//11010011
		data[1] = 0;
		data[2] = 0;
	
		ret = wiringPiSPIDataRW(TOUCH_CS, data, 3);
		printf("%d\n", data[1]);
		printf("%d\n\n", data[2]);
		for(int a = 0; a<100000000; a++);
	}
}