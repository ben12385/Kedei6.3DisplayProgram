import spidev
import time
import RPi.GPIO as GPIO
import random

#Disable Touchscreen
GPIO.setmode(GPIO.BCM)
GPIO.setup(8, GPIO.OUT, initial=GPIO.HIGH)


spi = spidev.SpiDev()
spi.open(0, 1)
spi.max_speed_hz = 32000000

#Reset display
def resetDisplay():
	global spi
	to_send1 = [0x00, 0x01, 0x00, 0x00]
	to_send = [0x00, 0x00, 0x00, 0x00]
	spi.writebytes(to_send1)
	time.sleep(0.1)
	spi.writebytes(to_send)
	time.sleep(0.1)
	spi.writebytes(to_send1)
	time.sleep(0.1)

def sendColour(colour):
	global spi
	GPIO.output(8, 1)
	toSend = [0x15, colour>>8, colour&0xFF]
	spi.writebytes(toSend)
	GPIO.output(8, 0)
	
def sendCommand(cmd):
	global spi
	GPIO.output(8, 1)
	toSend = [0x11, 0x00, cmd]
	spi.writebytes(toSend)
	GPIO.output(8, 0)
	
def sendData(data):
	global spi
	GPIO.output(8, 1)
	toSend = [0x15, 0x00, data]
	spi.writebytes(toSend)
	GPIO.output(8, 0)

def setFrame(x,y,w,h):
	sendCommand(0x2A)
	sendData(x>>8)
	sendData(x&0xFF)
	sendData(((w+x)-1)>>8)
	sendData(((w+x)-1)&0xFF)
	
	sendCommand(0x2B)
	sendData(y>>8)
	sendData(y&0xFF)
	sendData(((h+y)-1)>>8)
	sendData(((h+y)-1)&0xFF)

	sendCommand(0x2C)
	
	
	
resetDisplay()

time.sleep(0.01)

#NOP for what???
sendCommand(0x00)

#No idea what FF is for could be nothing
sendCommand(0xFF)
sendCommand(0xFF)
sendCommand(0xFF)
sendCommand(0xFF)
sendCommand(0xFF)
sendCommand(0xFF)

#Exit sleep mode, wait 5ms before seconding next command, load all default values
sendCommand(0x11)
time.sleep(0.05)

#RGB Interface Singal Control but did not select RBG interface
sendCommand(0xB0)
sendData(0x00)

#Set frame rate for partial mode which is weird as it is in normal mode, commented out, instant blackout
#sendCommand(0xB3)
#sendData(0x02)
#sendData(0x00)
#sendData(0x00)
#sendData(0x00)

#Set backlight control but send too many, trying with 1 parameter (nothing changed)
sendCommand(0xB9)
sendData(0xFF)
#sendData(0x00)
#sendData(0x0F)
#sendData(0x0F)

#Power control but send too many
sendCommand(0xC0)
sendData(0x13)
#sendData(0x3B)
#sendData(0x00)
#sendData(0x02)
#sendData(0x00)
#sendData(0x01)
#sendData(0x00)
#sendData(0x43)

#Step up circuit control, no idea what it is
sendCommand(0xC1)
sendData(0x08)
sendData(0x0F)
sendData(0x08)
sendData(0x08)

#Dont even have C4 for 9341 so assume it is 9340 which has it which is power for partial mode
sendCommand(0xC4)
sendData(0x11)
sendData(0x07)
sendData(0x03)
sendData(0x04)

#Dont even have C6 for 9340 so assume it is 9486 which has it for CABC Control
sendCommand(0xC6)
sendData(0x00)

#CABC Control, what is that??? Again, too many parameters
sendCommand(0xC8)
sendData(0x03)
sendData(0x03)
sendData(0x13)
sendData(0x5C)
sendData(0x03)
sendData(0x07)
sendData(0x14)
sendData(0x08)
sendData(0x00)
sendData(0x21)
sendData(0x08)
sendData(0x14)
sendData(0x07)
sendData(0x53)
sendData(0x0C)
sendData(0x13)
sendData(0x03)
sendData(0x03)
sendData(0x21)
sendData(0x00)

#Control tearing Effect Line, current set to 0, try 1 later
sendCommand(0x35)
sendData(0x00)

#Memory access control, currently set to 0110 0000 which sets MH to 1 thats all which means should be from right to left, to check vertical (no results, no matter what I put the way it refreshes doesnt change)
#If 0x00 is sent, the screen will fill 3/4 only, 
#Set to 60 to activate column address order(top down instead of bottom up) and row/column exchange(fill up whole screen instead of 3/4)
sendCommand(0x36)
sendData(0x60)

#Set pixel format, current set to 0101(DPI set to 16bits) 0101(CPU interface set to 16 bit as well) dont need to test as unable to use 18 bit
#Default is 06h which is 18bit
sendCommand(0x3A)
sendData(0x55)

#Adjusts tearing
sendCommand(0x44)
sendData(0x00)
sendData(0x01)

#NV Memory write, again too many parameters, both are required, random value also can output
sendCommand(0xD0)
sendData(0x07)
sendData(0x07)
sendData(0x1D)
sendData(0x03)

#NV memory protection, key is suppose to be 55AA66 for OTP programming
#sendCommand(0xD1)
#sendData(0x03)
#sendData(0x30)
#sendData(0x10)

#Read memory status but lacking 1 parameter
#sendCommand(0xD2)
#sendData(0x03)
#sendData(0x14)
#sendData(0x04)

#On the display
sendCommand(0x29)

#Display Inversion, 00 disables inversion
#sendCommand(0xB4)
#sendData(0x00)

#Write to memory
#sendCommand(0x2C)





#Make it black
print("Making it black")
setFrame(0,0,480,320)	

value = random.randint(0, 255*255)
for a in range(0, 480*320):
	sendColour(value)

print("Done")