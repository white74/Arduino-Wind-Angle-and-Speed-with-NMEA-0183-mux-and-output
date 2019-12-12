/* Bring in analog values 0-5vdc from wind sensors and output nmea 0183 
    Wind direction and speed sentence NMEA 0183
    $WIMWV
    $   String Start 
    WI  Weather Instrument
    MWV Wind Speed and Angle
        1 2 3 4 5
        | | | | |
$WIMWV,x.x,a,x.x,a*hh
1) Wind Angle, 0 to 360 degrees
2) Reference, R = Relative, T = True
3) Wind Speed
4) Wind Speed Units, K/M/N
5) Status, A = Data Valid
6) Checksum
*/

#include <SoftwareSerial.h>
#include "LedControl.h"
#define numReadings 10
// the number of readings from the analog input
  int readings[numReadings];
// the readings from the analog input
  int anglereadings[numReadings];
// the readings from the analog input
  int readIndex = 0;
// the index of the current reading
  int total = 0;
// the running total
  int angletotal = 0;
// the running total
  int average = 0;
// the average

  int v = 0;
  unsigned int offset = 0;      
//side of 8 digit display 0 left, 4 right 
#define DataIn 5
#define LOAD 6
#define CLK 7
#define MAX7219 2
  
  int anglePin = A0;
  int speedPin = A5;   
// wiper on angle sensor connected to analog pin 0
// speed sensor connected to analog pin 1
  float WindAngle  = 0.0;
  float WindSpeed = 0.0;
  String windir="";
  float appangle= 0.0;
  unsigned int angleValue = 0.0;
  unsigned int speedValue = 0.0;
  unsigned int Speed=0;
  unsigned int angle=100;
#define AngleCal -17.00
#define SpeedCal 2.000           
// Calibration addition wind angle sensor *10 
// Calibration multiplier wind speed sensor *10
  
  String Wind = "";
  String InNMEA = "";
  String Checksum ="";
#define WindSpeedUnit "N,"    
// unit for display N = Nautical Miles
#define  outRX 8
// nmea RX + pin 8
#define outTX 9
// nmea TX + pin 9 
#define interval 100
// interval at which to send NMEA Wind (milliseconds)
  unsigned long previousMillis = 0;     // will store last time NMEA Wind sent
  unsigned long currentMillis =  0;
  
  unsigned int led=2;
  unsigned int LEDIntensity=3;
  unsigned int ones=0;
  unsigned int tens=0;
  unsigned int hundreds=0;
  unsigned int tenths=0;
  unsigned int digits=0;
LedControl lc = LedControl (DataIn,CLK,LOAD,MAX7219);
SoftwareSerial NMEA(outRX, outTX); 

void setup() {
  Serial.begin(4800);
// initialize serial communication for NMEA out at 4800 bits per second:
  NMEA.begin(4800);
// initialize serial communication for NMEA in at 4800 bits per second:

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
    anglereadings[thisReading] = 0;
    }

  led=0;
  ledwakeup();
  led=1;
  ledwakeup();
}

void loop() {

if (Serial.available()) {                                                                     // If anything comes in Serial (USB),
  InNMEA = Serial.readStringUntil('\n');                                                      // read nmea in and output string
  NMEA.print(InNMEA);                                                                         //Print Incoming NMEA out to NMEA port
}       //if (Serial.available() close

currentMillis = millis();

if (currentMillis - previousMillis >= interval) {                                             //Checks time past since last send of Wind NMEA and if > interval sends annother NMEA string
  previousMillis = currentMillis;                                                             //sets time to new value  
    Speed = ReadSpeed();
    
// build the nmea 0183 string for wind instrument and add checksum 

  WindAngle  = (angle+AngleCal)/10.0;                                                         //Apply Angle Calibration Variable and /10 to get 1 dec place
  WindSpeed = (Speed*SpeedCal)/10.0;                                                          //Apply Speed Calibration Variable and /10 to get 1 dec place
  Wind = "WIMWV,";                                                                            // WI = Weather instrument MWV wind data string
  Wind=Wind+String(WindAngle, 1)+",R,"+String(WindSpeed, 1)+","+WindSpeedUnit+"A";            // Builds string data without line start, end, or Checksum to send to NMEA out port
  Checksum = Checksumcalc();
    Wind="$" + Wind + "*" + Checksum;                                                         // add line start and end with checksum
    // test value
    // WindAngle=190.0;
    
    if (WindAngle > 180) {
      windir="Port";
      offset=6;
      appangle=360-WindAngle;
      if (WindAngle > 240) {
       offset=7; 
      }
    }
    else if (WindAngle < 180) {
      windir="Stbd";
      appangle = WindAngle;
      offset=4;
      if (WindAngle < 120) {
       offset=3; 
      }
    }
    else {
      windir="DDW";
      appangle = WindAngle;
      offset=5;
      delay (500);
    }
    
    
    NMEA.println(Wind);                                                                       // print string to NMEA Output port
    lc.clearDisplay(0);
    lc.clearDisplay(1);
    led=0;
    v = appangle*10;
    printNumber(v);
    //delay (2000);
    led=1;
    offset=5;
    v = WindSpeed*10;
    printNumber(v);
    //delay (2000);
    
    Serial.print("Wind Speed\t");
    Serial.print(String(WindSpeed, 1));
    Serial.print("\tKnots\t");
    Serial.print("Wind Angle\t");
    Serial.print(String(appangle, 1));
    Serial.print("\t");
    Serial.println(windir);
    
}       
//if (currentMillis - previousMillis >= interval) close

}       
//loop close

int ReadSpeed(){   
   // subtract the last reading:
  total = total - readings[readIndex];
  angletotal = angletotal - anglereadings[readIndex];

  // read from the sensor:
  readings[readIndex] = analogRead(speedPin);
  anglereadings[readIndex] = analogRead(anglePin);
  delay(100);
  // add the reading to the total:
  total = total + readings[readIndex];
  angletotal = angletotal + anglereadings[readIndex];
  
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  speedValue = total / numReadings;
  angleValue = angletotal / numReadings;

  delay(1);
  // delay in between reads for stability   

  Speed = map(speedValue, 0, 1023, 0.0, 833.334);                                             // remaps speed value to 0 - 833.334 Knots*10
  angle = map(angleValue, 0, 1023, 3500, 100);                                            // remaps angle value to 100 - 3500 Degrees*10 
  
return Speed;
}

String Checksumcalc(){
  String data = Wind; 
  byte crc = 0;
  for(int i=0;i<data.length();i++){                   // creates Checksum for string
    crc=crc^data[i];
    Checksum=String(crc,HEX);
  }//checksum calc close
return Checksum;
}

int printNumber(int v) {
    if(v < -9999 || v > 9999) 
       return;
       if (v>999)digits=4;
       if (v<1000)digits=3;
       if (v<100)digits=2;
       if (v<10)digits=1;
    tenths=v%10;
    v=v/10;
    ones=v%10;
    v=v/10;
    tens=v%10;
    v=v/10;
    hundreds=v;
        
    //Now print the number digit by digit
    if (digits < 4){
      lc.setRow(led,7-offset+3,B00000000);
    }
    else {
      lc.setDigit(led,7-offset+3,(byte)hundreds,false);
    }
    if (digits < 3){
      lc.setRow(led,7-offset+2,B00000000);
    }
    else {
      lc.setDigit(led,7-offset+2,(byte)tens,false);
    }
    if (digits < 2){
      lc.setRow(led,7-offset+1,B10000000);
    }
    else {
      lc.setDigit(led,7-offset+1,(byte)ones,true);
    }
    lc.setDigit(led,7-offset+0,(byte)tenths,false);
    //lc.setDigit(led,7-offset+3,(byte)hundreds,false);
    //lc.setDigit(led,7-offset+2,(byte)tens,false);
    //lc.setDigit(led,7-offset+1,(byte)ones,true);
    //lc.setDigit(led,7-offset+0,(byte)tenths,false);
}

//The MAX72XX is in power-saving mode on startup,we have to do a wakeup call
void ledwakeup () {
  lc.shutdown(led,false);
// Wake up the max7219
  lc.setIntensity(led,LEDIntensity);
// Set the brightness to a values
  lc.clearDisplay(led);
// and clear the display
}
