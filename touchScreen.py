import spidev
import time
import RPi.GPIO as GPIO
import random
import numpy as np

GPIO.setmode(GPIO.BCM)
GPIO.setup(8, GPIO.OUT, initial=GPIO.HIGH)


spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 32000000