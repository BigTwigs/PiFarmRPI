import RPi.GPIO as GPIO
import time

relayPin = 20  #designation of GPIO pin for relay

#Setup of GPIO pin from relay
def setupGPIO(pin):
    GPIO.setmode(GPIO.BCM)                 
    GPIO.setup(pin,GPIO.OUT,initial=GPIO.HIGH) #set RelayPin's mode to output,and initial level to HIGH(5V) so the circuit remains open

def cleanUpGPIO():
    #Cleanup of GPIO when finished
    GPIO.cleanup()

#Check reading to determine if the plants need watering, i.e., no soil moisture detected and activate pump
#Function will return 1 if the pump was activated and 0 if it was not activated
def activatePump(moistureReading):
    if (moistureReading == 0):
        print('Moisture reading of 0 indicating no soil moisture detected\n')
        setupGPIO(relayPin)             #prepare GPIO to send signal to relay
        GPIO.output(relayPin,GPIO.LOW)  #closes circuit in relay and turns pump on
        print('Pumped turned on.\n')
        time.sleep(300)                 #Keeps circuit closed for 5 minutes allowing pump to be on for 5 minutes
        GPIO.output(relayPin,GPIO.HIGH) #reopens circuit in relay and turns off the pump 
        print('Pumped turned off after 5 minutes.\n')
        cleanUpGPIO()
        return 1
    else: 
        print('Plants do not need watering')
        return 0 
