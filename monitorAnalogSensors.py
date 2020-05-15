'''
This python script will run on reboot of the Raspberry Pi after the Arduino code is uploaded to its board. 
Its main purpose is to listen the port which facilitates serial communication between the two devices to do the following:
1) Send the time to the Arduino when a request for the time is received
2) Capture the ph and ppm values read from the sensors connected to the Arduino and send to the current user's collection on the database
'''
from serial import Serial
from datetime import timezone 
import datetime 

import RPi.GPIO as GPIO
import time

#imports for connectivity to Firebase Cloud Firestore on Raspberry Pi
import firebase_admin
import google.cloud
from firebase_admin import credentials, firestore   

#constants 
TIMEREQUEST = '$'
PHSIGNAL = '#'
PPMSIGNAL = '&'

#connection to firebase
cred = credentials.Certificate("/home/pi/PiFarmRPI/PiFarmServiceAccountKey.json")
app = firebase_admin.initialize_app(cred)
db = firestore.client()

#Setup of port to listen on for serial communication between the Raspberry Pi and the Arduino Uno
unoPort = Serial('/dev/ttyACM0', 9600)

#Getting the current date and time from the Raspberry Pi
def getTime():
    dt = datetime.datetime.now() 
  
    utc_time = dt.replace(tzinfo = timezone.utc) 
    utc_timestamp = utc_time.timestamp() 
  
    print(utc_timestamp)
    timeString = 'T' + str(utc_timestamp)
    timeString = timeString.encode('utf-8')
    return timeString

#Query collection in Firebase Cloud Firestore to get the ID of the user currently logged in to the webapp to gain access that user's collection
def getUserID():
    query_ref = db.collection(u'current user')
    query = query_ref.order_by(u'timestamp',direction = firestore.Query.DESCENDING).limit(1)
    querySnapshot = query.stream()
    for i in querySnapshot:  #will only produce one object as the limit was 1
        documentSnapshot = i

    if (documentSnapshot.exists):
        userID = documentSnapshot.get(field_path='userid')
        #print('User ID: ' + userID)   
    else:
        print('No user in database yet.')
    return userID

def sendToDatabase(userID: str, readingType: str, reading: str):
    try: 
        doc_ref = db.collection(u'users').document(userID).collection(readingType)
        doc_ref.add({u'value': str(reading),u'timestamp': firestore.SERVER_TIMESTAMP})
        print("Successfully added to database on ",datetime.datetime.now())    
    except:
        print("Error adding to the database.\n")

def main(): 
    while 1:
        if(unoPort.in_waiting > 0): #check if the input buffer has anything in the port
            msgBytes = unoPort.read()
            msg = msgBytes.decode()
            #print (msg)  #REMOVE
            if (msg == TIMEREQUEST): #check for the time request from Arduino
                timeString = getTime()
                unoPort.write(timeString)
                print('Time Sent\n')  

            if (msg == PHSIGNAL):   #check for the character on the serial monitor which suggests a ph reading is the following string
                data = unoPort.readline()   #read the ph reading string which is followed by a new line character eg. ph:4.00\n
                data = data.decode()
                print(data)
                if data.startswith('ph:'):
                    string = data.strip().split(':')
                    phReading = string[1]   #ph value will be the second part of the string
                    print(phReading)
                    uid = getUserID()     #in the event user accounts are switched between readings
                    sendToDatabase(uid,'ph',phReading)
                    

            if (msg == PPMSIGNAL):   #check for the character on the serial monitor which suggests a ppm reading is the following string
                data = unoPort.readline()  #read the ppm reading string which is followed by a new line character eg. ppm:434.45\n
                data = data.decode()
                print(data)
                if data.startswith('ppm:'):
                    string = data.strip().split(':')
                    ppmReading = string[1]   #ppm value will be the second part of the string
                    print(ppmReading)
                    uid = getUserID()     #in the event user accounts are switched between readings
                    sendToDatabase(uid,'ppm',ppmReading)


if __name__ == "__main__":
    main()