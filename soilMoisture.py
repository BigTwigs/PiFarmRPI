import RPi.GPIO as GPIO
from statistics import mode
import datetime

#python script written to activate pump by sending a signal to a relay to close a circuit
import pump

#imports for connectivity to Firebase Cloud Firestore on Raspberry Pi
import firebase_admin
import google.cloud
from firebase_admin import credentials, firestore   

#connection to firebase
cred = credentials.Certificate("/home/pi/PiFarmRPI/PiFarmServiceAccountKey.json")
app = firebase_admin.initialize_app(cred)
db = firestore.client()

pin = 21  #designation of GPIO pin for communication with soil moisture sensor

#Setup of GPIO pin for soil moisture sensor
def setupGPIO(pin):
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(pin,GPIO.IN)

'''
GPIO.HIGH indicates no moisture detected giving a 5v signal and will be represented in the database as a 0
GPIO.LOW indicates moisture has been detected giving a 0v signal and will be represented in the database as a 1 
'''

#Take reading from soil moisture sensor
#10 readings are taken and the modal value will be used to ensure accuracy of the reading is maintained
def takeReading():
    readings = []
    for i in range(0,10): 
        if (GPIO.input(pin) == GPIO.LOW):   #when moisture is detected
            readings.append(1)
        else: 
            readings.append(0)
    reading = mode(readings)
    if reading == 0:
        print('Reading: ' + str(reading) + ' indicating no soil moisture detected.\n')
    else:
        print('Reading: ' + str(reading) + ' indicating soil moisture detected.\n')
    return reading

#Cleanup of GPIO when finished
def cleanUpGPIO():
    GPIO.cleanup()

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

#Sending reading to the current user's collection in Firebase Cloud Firestore using the User ID obtained 
def sendToDatabase(reading,userID):
    try: 
        doc_ref = db.collection(u'users').document(userID).collection(u'moisture')
        doc_ref.add({u'value': str(reading),u'timestamp': firestore.SERVER_TIMESTAMP})
        print("Successfully added to database on ",datetime.datetime.now())    
    except:
        print("Error adding to the database\n")

#Sending timestamp of watering to the Firebase Cloud Firestore 
def sendTimestampToDatabase(userUID: str):
    try: 
        doc_ref = db.collection(u'users').document(userUID).collection(u'last watered')
        doc_ref.add({u'timestamp': firestore.SERVER_TIMESTAMP})
        print("Successfully added to timestamp to database at ", datetime.datetime.now())    
    except:
        print("Error adding to the database\n")

def main():
    setupGPIO(pin)
    uid = getUserID()
    moistureReading = takeReading()
    sendToDatabase(moistureReading,uid)
    cleanUpGPIO()
    watered = pump.activatePump(moistureReading)
    if (watered == 1):  #return value of 1 means the pump has been activated
        sendTimestampToDatabase(uid)
        sendToDatabase(1,uid)


if __name__ == "__main__":
    main()