#include <SoftwareSerial.h>
SoftwareSerial NMEA(8, 9); // RX, TX 
  int anglePin = A0;                     // wiper on angle sensor connected to analog pin 0
  int speedPin = A0;                     // speed sensor connected to analog pin 1
  float WindAngle  = 0.0;
  float WindSpeed = 0.0;
  String InNMEA = "";
  String Checksum ="";
  int angleValue = 0.0;
  int speedValue = 0.0;
  int Speed=0.0;
  int angle=0.0;
  String Wind = "";
  const String WindSpeedUnit = "N,";    // unit for display N = Nautical Miles
  const long interval = 800;            // interval at which to send NMEA Wind (milliseconds)
  unsigned long previousMillis = 0;     // will store last time NMEA Wind sent
  const int AngleCal = 0;               // Calibration of wind angle sensor *10 
  const int SpeedCal = 0;               // Calibration of wind speed sensor *10
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
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(4800);
  NMEA.begin(4800);
}
// the loop routine runs over and over again forever:
void loop() {
if (Serial.available()) {                                                                     // If anything comes in Serial (USB),
  InNMEA = Serial.readStringUntil('\n');                                                      // read nmea in and output string
  NMEA.print(InNMEA);                                                                         //Print Incoming NMEA put to NMEA port
}       //if (Serial.available() close
  unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= interval) {                                             //Checks time past since last send of Wind NMEA and if > interval sends annother NMEA string
  previousMillis = currentMillis;                                                             //sets time to new value  
    Speed = ReadSpeed();
    angle = ReadAngle();  
// build the nmea 0183 string for wind instrument and add checksum 
  Wind = "WIMWV,";                                                                            // WI = Weather instrument MWV wind data 
  WindAngle  = (angle+AngleCal)/10.0;                                                         //Apply Angle Calibration Variable and /10 to get 1 dec place
  WindSpeed = (Speed+SpeedCal)/10.0;                                                          //Apply Speed Calibration Variable and /10 to get 1 dec place
  Wind=Wind+String(WindAngle, 1)+",R,"+String(WindSpeed, 1)+","+WindSpeedUnit+"A";            // Builds string data without line start, end, or Checksum to send to NMEA out port
  Checksum = Checksumcalc();
    Wind="$" + Wind + "*" + Checksum;                                                         // add line start and end with checksum
    NMEA.println(Wind);                                                                       // print string to NMEA Output port
}       //if (currentMillis - previousMillis >= interval) close
}       //loop close
int ReadSpeed(){                                       
  speedValue = analogRead(speedPin);                                                          // Reads Value from Analog pin 1 for wind speed
  Speed = map(speedValue, 0, 1023, 0.0, 833.334);                                             // remaps speed value to 0 - 833.334 Knots*10
return Speed;
}
int ReadAngle(){
  angleValue =analogRead(anglePin);                                                           // Reads Value from Analog pin 0 for wind angle
  angle = map(angleValue, 0, 1023, 3500.0, 100.0);                                            // remaps angle value to 100 - 3500 Degrees*10    
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
