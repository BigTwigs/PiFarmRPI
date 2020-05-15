/*
 * The design pattern implemented in this sketch is referred to as Proto-threading and allows for multiple jobs 
 * on the Arduino Uno to be done. The least-blocking action is contained within the main loop.
 * The actions completed in this sketch are as follows:
 * 1. Syncs the Arduino's clock with a timestamp(s) received from Raspberry Pi (Least-blocking)
 * 2. Takes readings from the pH sensor and sends it via serial communication to the Raspberry Pi at particular times of the day.
 * 3. Takes readings from the turbidity sensor and sends it via serial communication to the Raspberry Pi at particular times of the day.
 * 
 * Note: all unecessary prints to the Serial to test have either been removed or commented.
 * Only necessary communication on the serial line is reflected in this code. 
 * The Serial Monitor has been used to verify the code during testing.
 * 
 * The code contained within this sketch has been modelled after sample code found on the internet;
 * particularly from the manufacturer's of the sensors and the documentation of the Time library.
 * These sources have been cited in the documentation for this project.
 */

#include <TimeLib.h>
#include <Wire.h>   //needed for Action 3 

//Variables used for Action 1: Time Sync
char TIME_HEADER = 'T';   // Header tag for serial time sync message
char TIME_REQUEST = '$';   // ASCII bell character requests a time sync message 
int temp;             //used to catch the return value of the requestSync function which is always 0

//Variables used for Actions 2 and 3
time_t t;
bool phReadingTakenAt11 = false;
bool ppmReadingTakenAt11 = false;
bool phReadingTakenAt3 = false;
bool ppmReadingTakenAt3 = false;  

#define phSensorPin A0          //pH meter Analog input/output to Arduino Analog Pin A0 
#define ppmSensorPin A1         //turbidity sensor Analog input/output to Arduino Analog Pin A1
float phReading;
float ppmReading;
char PH_SIGNAL = '#';
char PPM_SIGNAL = '&';

void setup() {
  Serial.begin(9600);
  
  //for testing
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  //Action 1: Time Sync
  //temp = requestSync();   //request time from external source, i.e., raspberry pi 
}

void loop() {
  
  //Action 1: Least Blocking 
  if (timeStatus() == timeNotSet || timeStatus() == timeNeedsSync){
    temp = requestSync();
    delay(1000);  
  }
  if (Serial.available()) {
    processSyncMessage();
  }

  /*
  //Can be used to display the time in the serial monitor NB: not recommended to flood the serial line
  if (timeStatus()!= timeNotSet) {
    //digitalClockDisplay(); 
  }
  */
  //delay(1000);

  t = now();  //records the current time in the format time_t 

  //Take sensor readings at 11am
  if (hour(t) == 11 && minute(t) == 0){  //11am
      //if ph flag is false, take reading by calling function, send on serial line, set ph flag
      
      if (!phReadingTakenAt11) {
          phReading = takePhReading();
          //send on serial line, using '#' character to indicate the string after is the ph reading followed by a newline character
          Serial.print(PH_SIGNAL);
          Serial.print("ph:");  
          Serial.println(phReading,2);  //pH reading with 2 decimal places followed by newline character
          phReadingTakenAt11 = true;
      }   
      
      //if ppm flag is false, take reading by calling function, send on serial line, set ppm flag
      if (!ppmReadingTakenAt11) {
          ppmReading = takePpmReading();
          //send on serial line, using '&' character to indicate the string after is the ppm reading followed by a newline character
          Serial.print(PPM_SIGNAL);
          Serial.print("ppm:");  
          Serial.println(ppmReading,2);  //ppm reading with 2 decimal places followed by newline character
          ppmReadingTakenAt11 = true; 
      } 
    } 

  //Take sensor readings at 3pm
  if (hour(t) == 3 && minute(t) == 0) {  //3pm
      //if ph flag is false, take reading by calling function, send on serial line, set ph flag
      if (!phReadingTakenAt3) {
          phReading = takePhReading();
          //send on serial line, using '#' character to indicate the string after is the ph reading followed by a newline character
          Serial.print(PH_SIGNAL);
          Serial.print("ph:");  
          Serial.println(phReading,2);  //pH reading with 2 decimal places followed by newline character
          phReadingTakenAt3 = true; 
      }

      //if ppm flag is false, take reading by calling function, send on serial line, set ppm flag
      if (!ppmReadingTakenAt3) {
          ppmReading = takePpmReading();
          //send on serial line, using '&' character to indicate the string after is the ppm reading followed by a newline character 
          Serial.print(PPM_SIGNAL);
          Serial.print("ppm:");  
          Serial.println(ppmReading,2);  //ppm reading with 2 decimal places followed by newline character
          ppmReadingTakenAt3 = true; 
      }
    }

    //Reset flags for readings taken at 11am and 3pm at 1am each day
    if (hour(t) == 1 && minute(t) == 0) { //1am
        phReadingTakenAt11 = false;
        ppmReadingTakenAt11 = false;
        phReadingTakenAt3 = false;
        ppmReadingTakenAt3 = false;
    }

    /*
     * The Arduino is not performing the vital functions for which it is used for, i.e, to take readings from the sensors
     * outside the time slot of 11am and 3am promptly. If an external clock such as a RTC module was used, 
     * we could have put the Arduino to sleep thus saving power as the Arduino needs an external interrupt to wake up.
     * We are using the time from the raspberry pi, therefore we minimize the logic to be performed by the Arduino 
     * as it constantly loops thus reducing the amount of blocking work and less processing power.
     */
}

//Functions used for Action 1: Time Sync

//digitalClockDisplay() and printDigits() are for displaying the timestamp on the serial monitor
/*
void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year())
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
*/

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
     }
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);  //send bell request on serial line
  return 0; // the time will be sent later in response to serial mesg
}

//Functions for Action 2: Reading from pH sensor

float takePhReading(){
 unsigned long int avgValue;    //Store the average value of the sensor feedback
 int buf[10],temp;
 for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(phSensorPin);
    delay(10);                
  }

  //Code which will derive the most acccurate value from 10 readings
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  avgValue = avgValue/6;
    
  float phValue=(float)avgValue*5.0/1024;   //convert the analog average into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  return phValue;
}

//Functions for Action 3: Reading from Turbidity sensor
float takePpmReading() {
    //takes the average of 10 readinfs to give the most accurate reading from the turbidity sensor
    float volt = 0;
    for(int i=0; i<10; i++)
    {
        volt += ((float)analogRead(ppmSensorPin)/1023)*5;
        //Serial.println(volt);
    }
    volt = volt/10;
    volt = round_to_dp(volt,1); 
    float ppm = volt_to_ppm(volt);
    return ppm;
} 

float round_to_dp(float in_value, int decimal_place) {
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

float volt_to_ppm(float volt) {
  //linear equation given on the turbidty sensor's official website with an adjustment made 
  //due to the fact that when the voltage is 4.7, the NTU value is -2113.73 using the linear equation given 
  //thus the equation has been modified to reflect 0NTU when the voltage is 4.7
  float ntu = (-1120.4*square(volt))+(5742.3*volt)-4352.9+2113.73;

  //According to the manufacturer's website, 1 NTU = 1 mg/l (1 milligram per litre)
  //Based on the concentration of chemicals in water, i.e., the nutrient solution, 1 mg/l = 1 ppm
  //thus 1 NTU = 1 ppm so we can return the ntu value as the ppm value
  return ntu;
}
