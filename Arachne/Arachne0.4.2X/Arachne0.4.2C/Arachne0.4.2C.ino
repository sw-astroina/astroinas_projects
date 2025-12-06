/*
Arachne 0.4.2C
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
modified again by me, obviously
*/
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
int genSeed;  //this is what the dht humidity data turns into, to generate bools from

void setup() {
  Serial.begin(9600);
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
    Serial.println(", bool = 0");
  }
  else {
    Serial.println(", bool = 1");
  }

  delay(500);
}


/*
Like half of this is my own sketchy variable-type-converting code, so we'll see how this goes. 

RESULT: Holy shit it works! 
Good for me! I was pretty sure this code was gonna be one of the problems my bigger program has, but evidently not!

This indicates that the DHT is connected properly and I can pull a semi-random bool value out of its ass.
*/