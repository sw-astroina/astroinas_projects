/*
Arachne 0.4.2D
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
  No ReadMe in code, but stolen from 
  adafruit_aht_test in the AdafruitAHTX0 library

and 
 Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 https://www.arduino.cc/en/Tutorial/LibraryExamples/Sweep
modified again by me, obviously
*/

#include <Adafruit_AHTX0.h>

#include <Servo.h>

Servo myservo1;  // create servo object to control a servo
Servo myservo2;
// twelve servo objects can be created on most boards   //SophieNote: insert crying emoji here

Adafruit_AHTX0 aht;
int genSeed;  //this is what the dht humidity data turns into, to generate bools from
int posUp = 0;    // variable to store the servo position
int posDown = 180;

void setup() {
  Serial.begin(9600);
  myservo1.attach(5);  // attaches the servo on pin 5 to the servo object
  myservo2.attach(6); 
  Serial.println("Adafruit AHT10/AHT20 demo!");

  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  genSeed = 100 * humidity.relative_humidity; //SophieNote: You can't % floats, apparently, so genSeed turns humidity.relative_humidity into an int
  Serial.print("Temperature: "); Serial.print(temp.temperature);
  Serial.print(", Humidity: "); Serial.print(humidity.relative_humidity); 

  if (genSeed % 2 == 0) { //SophieNote: Checking even or odd for the last digit
    myservo1.write(posUp);              // tell servo to go to position in variable 'pos'
    myservo2.write(posDown);
    Serial.println(", bool = 0");
  }
  else {
    myservo1.write(posDown);              // tell servo to go to position in variable 'pos'
    myservo2.write(posUp);
    Serial.println(", bool = 1");
  }

  delay(1500);
}

/*
In the past with only 2a, the DHT struggled to get enough power when the servos were eating it all.
My computer's powering the microcontroller while the 5V 20a supply's powering the servos (with connected grounds, of course)

RESULT: success!
I had to up the delay (should figure out a timeDeltaTime thing, so it doesn't pause the whole code in the future),
but it works just fine! The power supply seems to have fixed the previous issue :)

This indicates that I need 1.5 seconds for the servos to apply their motion, but the power thing is good to go (insert thumbs up emoji)
*/