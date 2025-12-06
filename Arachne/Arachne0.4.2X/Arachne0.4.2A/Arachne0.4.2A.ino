/*
Arachne 0.4.2A
Sophie Weidmann Dec 5 2025

The 0.4.2X versions are little chunks of code, made 
to test specific functionalities of the larger program. 
See 0.4.1 for context.

What are we testing?
A: controlling 2 servos at once
  B: reading the 3 switch inputs
  C: reading the DHT20 and turning the recieved float into a bool
  D: controlling the two servos based on the DHT's data
  E: holding and updating a 6x6 matrix array's worth of memory

Evidence of my swashbucklery and brigandage:
 Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 https://www.arduino.cc/en/Tutorial/LibraryExamples/Sweep
modified again by me, obviously
*/

#include <Servo.h>

Servo myservo1;  // create servo object to control a servo
Servo myservo2;
// twelve servo objects can be created on most boards   //SophieNote: insert crying emoji here

int pos1 = 0;    // variable to store the servo position
int pos2 = 180;

void setup() {
  myservo1.attach(5);  // attaches the servo on pin 9 to the servo object
  myservo2.attach(6); 
}

void loop() {
  for (pos1 = 0; pos1 <= 180;) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    pos1 += 1;
    pos2 -= 1;
    myservo1.write(pos1);              // tell servo to go to position in variable 'pos'
    myservo2.write(pos2);
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
  for (pos1 = 180; pos1 >= 0;) { // goes from 180 degrees to 0 degrees
    pos1 -= 1;
    pos2 += 1;
    myservo1.write(pos1);              // tell servo to go to position in variable 'pos'
    myservo2.write(pos2);             // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
}

/*
RESULT: success! 
servo2 on pin 6 lags a tiny bit behind servo1 when on computer power 
instead of the power supply, which was my mistake. They work together 
just fine, especially when properly powered by the 5V 20A supply. 

This indicates that the arduino can talk to both servos at once, 
and that the power supply won't struggle to keep up the way my 
5V 2a supply did. Not unexpected, but still a pleasing result!
*/