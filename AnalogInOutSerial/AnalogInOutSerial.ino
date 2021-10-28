/*
  Analog input, analog output, serial output

 Reads an analog input pin, maps the result to a range from 0 to 255
 and uses the result to set the pulsewidth modulation (PWM) of an output pin.
 Also prints the results to the serial monitor.

 The circuit:
 * potentiometer connected to analog pin 0.
   Center pin of the potentiometer goes to the analog pin.
   side pins of the potentiometer go to +5V and ground
 * LED connected from digital pin 9 to ground

 created 29 Dec. 2008
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */
#include <MPU6050_tockn.h>
#include <Wire.h>

#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 6); // RX, TX

MPU6050 mpu6050(Wire);
int Zangle;

// These constants won't change.  They're used to give names
// to the pins used:
//int pins[] = {3, 4, 12, 13, 14, 28, 29, 30, 31, 32 ,33 ,34, 35, 41, 42, 43, 44};
int adcpin[] = {A0, A1, A2, A3, A6};
int adcdata[] = {0, 1, 2, 3, 6}; 
int digidata_1;
int digidata_2;  

void setup() {
  // initialize serial communications
  Serial.begin(115200);
  mySerial.begin(115200);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  
  //pinMode(2,INPUT);
  //pinMode(3,INPUT);
  /*for(int i = 0 ; i < 16; i++)
  { 
    pinMode(pins[i], OUTPUT);
  }*/
}

void loop() {
  // read the analog in value:
  mpu6050.update();
  Zangle = mpu6050.getAngleZ();
  
  for(int a = 0; a < 5; a++)
  {
        adcdata[a] = analogRead(adcpin[a]);
  }

  //Serial.print("{");
  //Serial.print("\"AngleZ\":");
  Serial.println(Zangle);
  mySerial.println(Zangle);
  //Serial.println("}");

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  //delay(100);
}
