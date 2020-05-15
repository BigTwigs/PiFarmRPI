# PiFarmRPI
The code scripts for the Embedded System component of the PiFarm product. 

The code contained in this repository are those used to implement the functionality of the Embedded System component of the PiFarm Project. 
The readings taken from the sensors are scheduled to be taken at 11am and 3pm daily. For the sensors connected to the Arduino Uno, this is facilitated by the internal clock being set from a timestamp sent by the monitorAnalogSensors.py script. For the soil moisture sensor that is connected directly to the Raspberry Pi, this scheduling is facilitated using cron jobs. 

The monitorAnalogSensor.py script is a program consistently listening on the port to which serial communication between the Raspberry Pi and the Arduino Uno is facilitated. It listens for time request when the Arduino's internal clock is out of sync or not set as well as readings taken from the sensors. 

All readings are sent to specific user collections in the Firebase Cloud Firestore Database for this project which the web application also has access to.

Note: Readings taken with this embedded system can only be sent to one user's collection at a time; this will be the user collection of the user who is currently logged into the application and will change if another user account is logged in. 



To utilize this code on your own setup of the physical parts of the embedded system, you must do the following steps:

1. Save the PiFarmRPI folder in the /home/pi directory on your Raspberry Pi.
2. Open the analogSensors.ino file on the Arduino IDE (ensure you have this installed first) and upload the code to the Arduino Uno Board

Note: The Arduino Uno will continuously run this code without failure even if power is lost and it has been restarted

3. You can perform either of the following options:
    a. Run the monitorAnalogSensors.py 
       Setup the cron jobs to run the scripts on a schedule to enable the automation part of the system (instructions below)
       
    OR
    
    b. Setup the cron jobs to run the scripts on a schedule to enable the automation part of the system (instructions below)
       Reboot Raspberry Pi -> This should launch the monitorAnalogSensors.py file on reboot 
    
## To setup the cron jobs on the Raspberry Pi 3

1. run `crontab -e` command in your terminal
2. Choose the nano option to edit
3. Copy the contents of the cronjob.txt file at the bottom of the cron file that is open in the editor
4. To save, click `^X Exit`. You will then be prompted to save changes, choose yes and then hit enter when the file name to write is presented.

## To test this code without the scheduling:
You can run the soilMoisture.py file which will take readings from the soil moisture sensor, update the user collection in the database and activate the pump to water the plants automatically if no moisture is detected.

You can also edit the times in the analogSensors.ino file, upload the script to the Arduino Uno and then run the monitorAnalogSensors.py file. This will take readings from the pH and turbidity sensors and update the user collection in the database.




       
       



