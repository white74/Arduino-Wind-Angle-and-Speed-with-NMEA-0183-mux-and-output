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

  int v = 0;
  unsigned int offset = 0;             
//side of 8 digit display 0 left, 4 right 
  const unsigned int DataIn = 12;
  const unsigned int CLK = 11;
  const unsigned int LOAD = 10;
  const unsigned int MAX7219 = 2;
   
  int anglePin = A0;
  int speedPin = A1;   
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
  const int AngleCal = 0;
  const int SpeedCal = 0;           
// Calibration of wind angle sensor *10 
// Calibration of wind speed sensor *10
  
  String Wind = "";
  String InNMEA = "";
  String Checksum ="";
  const String WindSpeedUnit = "N,";    // unit for display N = Nautical Miles
  const unsigned int outRX = 8;
  const unsigned int outTX = 9;                   // RX pin 8, TX pin 9 

  const unsigned int interval = 500;     // interval at which to send NMEA Wind (milliseconds)
  unsigned long previousMillis = 0;     // will store last time NMEA Wind sent
  unsigned long currentMillis =  0;
  
  unsigned int led=1;
  unsigned int LEDIntensity=2;
  unsigned int ones=0;
  unsigned int tens=0;
  unsigned int hundreds=0;
  unsigned int tenths=0;

LedControl lc = LedControl(DataIn,CLK,LOAD,MAX7219);
SoftwareSerial NMEA(outRX, outTX); 

void setup() {
  Serial.begin(4800);
// initialize serial communication for NMEA out at 4800 bits per second:
  NMEA.begin(4800);
// initialize serial communication for NMEA in at 4800 bits per second:
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
    angle = ReadAngle();
    
// build the nmea 0183 string for wind instrument and add checksum 

  WindAngle  = (angle+AngleCal)/10.0;                                                         //Apply Angle Calibration Variable and /10 to get 1 dec place
  WindSpeed = (Speed+SpeedCal)/10.0;                                                          //Apply Speed Calibration Variable and /10 to get 1 dec place
  Wind = "WIMWV,";                                                                            // WI = Weather instrument MWV wind data string
  Wind=Wind+String(WindAngle, 1)+",R,"+String(WindSpeed, 1)+","+WindSpeedUnit+"A";            // Builds string data without line start, end, or Checksum to send to NMEA out port
  Checksum = Checksumcalc();
    Wind="$" + Wind + "*" + Checksum;                                                         // add line start and end with checksum
    // test value
    //WindAngle=190.0;
    
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
    led=1;
    offset=5;
    v = WindSpeed*10;
    printNumber(v);

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
  speedValue = analogRead(speedPin);                                                          // Reads Value from Analog pin 1 for wind speed
//  Speed = map(speedValue, 0, 1023, 0.0, 833.334);                                             // remaps speed value to 0 - 833.334 Knots*10
  
   if (Speed < 833) {
    Speed = Speed + 1;
  }
  else {
    Speed = 1;
  }
  
return Speed;
}

int ReadAngle(){
  angleValue =analogRead(anglePin);                                                           // Reads Value from Analog pin 0 for wind angle
//  angle = map(angleValue, 0, 1023, 3500.0, 100.0);                                            // remaps angle value to 100 - 3500 Degrees*10 
  
  if (angle < 3500) {
    angle = angle + 1;
  }
  else {
    angle = 100; 
  }

  return angle;
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
    tenths=v%10;
    v=v/10;
    ones=v%10;
    v=v/10;
    tens=v%10;
    v=v/10;
    hundreds=v;     
    
    //Now print the number digit by digit
    lc.setDigit(led,7-offset+3,(byte)hundreds,false);
    lc.setDigit(led,7-offset+2,(byte)tens,false);
    lc.setDigit(led,7-offset+1,(byte)ones,true);
    lc.setDigit(led,7-offset+0,(byte)tenths,false);
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
