/*
Arachne 1.0
Sophie Weidmann Dec 7 2025

----------------

This code is for a servo-controlled dobby loom I call Arachne, because of both its and my hubris! 
The loom is capable of both leading and following, in that it can lift the shed for the pattern you give it, or it can act independently.
Currently it's capable of designing a random weave structue, edited for fabric stability. 
This code is for 8 harnesses, but should be fine to test two for now and expand later.

----------------

Evidence of my swashbucklery and brigandage:
  Sweep by BARRAGAN <http://barraganstudio.com>
  modified 8 Nov 2013 by Scott Fitzgerald
  This example code is in the public domain.
  https://www.arduino.cc/en/Tutorial/LibraryExamples/Sweep

  created 2005 by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011 by Tom Igoe
  This example code is in the public domain.
  https://docs.arduino.cc/built-in-examples/digital/Button/
  
  No ReadMe in code, but stolen from 
  adafruit_aht_test in the AdafruitAHTX0 library

  For loops were stolen from texts from Max Weidmann
  Mwahahaha stealing from siblings >:]
modified by me, obviously
*/



#include <Adafruit_AHTX0.h>
#include <Servo.h>

Adafruit_AHTX0 aht;

Servo s1;  //Fitzgerald says twelve servo objects can be created on most boards
Servo s2;  //The uno has pins for 6, and I currently only own two.
Servo s3;  //I think eventually I'll get a servo driver to handle this
Servo s4;
Servo s5;
Servo s6;
Servo s7;
Servo s8;

//Pin Declarations//
const int pedalPin = 8;  
const int toggleUp = 12;
const int toggleDown = 13;  //there's a central "off" position to this toggle switch, too

//Memory Variables//
bool memory [8] [8]; //We need to remember what we've recently woven to prevent floats. 
                    //Not number floats, but threads that float above the fabric and compromise the structural stability.
                    //Essentially we're just checking that it's not all 0s or all 1s in any given row or column.

bool columnChange;  //these are for shuffling things around in the matrix. 
int changedColumn;
int changingRow;
int a;  //Max named a and b, and I'm too lazy to rename for clarity.
int b;  //they're essentially just counters for navigating the matrix

//Switches// 
bool prevPedalState = LOW; 
bool pedalState = LOW;  //these are for the status of the switches
bool upState = LOW;
bool downState = LOW;

//Miscellaneous//
bool on;         //self-explanatory. Is the switch set to "off"?
bool glitch;     //if the loom should generate its own pattern, glitch is true
bool prevGlitch = false; //so we can check pattern validity when transitioning out of glitch mode

int genSeed;     //this is what the dht humidity data turns into, to generate bools from
int pick = 0;    //tracking how many rows we've woven
int amends = 0;  //how many times has the pattern been changed during weavechecking? Just for display, to see what's happening under the hood

int shedUp = 180;    //these tell the servos where to go. A servo going down will pull threads up, so they're named for the threads' position, not the servos.
int shedDown = 0;

/////////////////////////////////

void setup() {
  Serial.begin(9600);

  if (! aht.begin()) {
    Serial.println("Could not find DHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("DHT20 found");

//Declare pins//
  pinMode(pedalPin, INPUT); //switch pins
  pinMode(toggleUp, INPUT);
  pinMode(toggleDown, INPUT);

  s1.attach(3); //servo pins
  s2.attach(5);
  s3.attach(6);
  s4.attach(9);
  s5.attach(10);
  s6.attach(11);
  s7.attach(2);  //7 and 8 are dummy pins, since the uno only has 6 PWM pins
  s8.attach(4);
}

/////////////////////////////////

void loop() {
  pedalState = digitalRead(pedalPin); //read the sensors
  upState = digitalRead(toggleUp);
  downState = digitalRead(toggleDown);

//Detect falling edge of pedal//
  if (pedalState == LOW && prevPedalState == HIGH) {
    if (upState == HIGH) {
      Serial.print("Patterning, ");
      glitch = false;
      on = true;
    } else {
      if (downState == HIGH) {
      Serial.print("Glitching,  ");
      glitch = true;
      on = true;
      } else {
      Serial.println("Off");
      on = false;
      delay(1000);
      }
    }


//Follow pattern//
    if (on == true) {
      if (glitch == false) {
        if (pick % 2 == 0) {
          memory [0] [0] = 1;
          memory [0] [1] = 0;
          memory [0] [2] = 1;
          memory [0] [3] = 0;
          memory [0] [4] = 1;
          memory [0] [5] = 0;
          memory [0] [6] = 1;
          memory [0] [7] = 0;
        } else {
          memory [0] [0] = 0;
          memory [0] [1] = 1;
          memory [0] [2] = 0;
          memory [0] [3] = 1;
          memory [0] [4] = 0;
          memory [0] [5] = 1;
          memory [0] [6] = 0;
          memory [0] [7] = 1;
        }
        if (prevGlitch == true) {
          weavecheck();
        }
        else {
          apply(); //skip weavecheck, under assumption that weaver wants what they want regardless of structural validity
        }
  /*
  Eventually, the setting of the dial potentiometer should determine which pattern is selected.
  Here's some pattern samples for reference. These are edited for 6 harnesses and to the nearest repeat.
  
  plain         basket       s twill        7-1 satin
  01010101      00110011     11001100       01000000
  10101010      00110011     01100110       00001000
                11001100     00110011       00000001
                11001100     10011001       00100000
                                            00000100                
                                            10000000
                                            00010000
                                            00000010

  Weave structures (how the fabric looks) aren't necessarily synonymous with shed patterns (how the servos move).
  */

//Generate new row//
      } else {
        for (a = 0; a < 7; a++) {
          sensors_event_t humidity, temp;
          aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
          genSeed = 100 * humidity.relative_humidity; //You can't % floats, apparently, so genSeed turns humidity.relative_humidity into an int
          //Serial.print("Temperature: "); Serial.print(temp.temperature);
          //Serial.print(", Humidity: "); Serial.print(humidity.relative_humidity); 

          if (genSeed % 2 == 0) { //Checking even or odd for the last digit
            memory [0] [a] = 0;
          }
          else {
            memory [0] [a] = 1;
          }
        }
        weavecheck();
      }
    } else {
      s1.write(shedDown); //if the toggle's set to "off," release tension
      s2.write(shedDown);
      s3.write(shedDown);
      s4.write(shedDown);
      s5.write(shedDown);
      s6.write(shedDown);
      s7.write(shedDown);
      s8.write(shedDown);
    }
  }
  prevPedalState = pedalState; // Update previous pedal state for next loop
}

/////////////////////////////////

void weavecheck() {
  amends = 0;
//Check warp/column//
  columnChange = false;
  for (a = 0; a < 7; a++) {
    if (memory [0] [a] == memory [1] [a]) {
      if (memory [0] [a] == memory [2] [a]) {
        if (memory [0] [a] == memory [3] [a]) {
          if (memory [0] [a] == memory [4] [a]) {
            if (memory [0] [a] == memory [5] [a]) {
              if (memory [0] [a] == memory [6] [a]) {
                if (memory [0] [a] == memory [7] [a]) {
                  if (memory [0] [a] == 0) {
                    memory [0] [a] = 1; //if the whole column is 0s, change the top digit to be a 1.
                    amends ++;
                  }
                  else {
                    memory [0] [a] = 0; //if the whole column is 1s, change the top digit to be a 0.
                    amends ++;
                  }
                  columnChange = true;
                  changedColumn = a; // I can only mark one column as being changed at a time. How to fix?
                }
              }
            }
          }
        }
      }
    }
  } //are nesting ifs bad practice? I think this one's readable, but then again I'm the one who wrote it

//Check weft/row//
  if (columnChange == true) {
    changingRow = random(0,8);
    if (changingRow == changedColumn) {
      changingRow++;
    }
    if (changingRow > 7) {
      changingRow = 0;   //if a column was changed, and the row still needs to be changed (1.2% chance), 
                         //don't change the rightmost column that changed. Imperfect system...
    } 
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] + memory [0] [6] + memory [0] [7] == 0) {
      memory [0] [changingRow] = 1; //if the whole row is 0s, change the 0 to the right of the last changed column to be a 1.
      amends ++;
    }
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] + memory [0] [6] + memory [0] [7] == 8) {
      memory [0] [changingRow] = 0; //if the whole row is 1s, change the 0 to the right of the last changed column to be a 1.
      amends ++;
    }
  } else {
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] + memory [0] [6] + memory [0] [7] == 0) {
      memory [0] [random(0,8)] = 1; //if the whole row is 0s, add a 1 somewhere.
      amends ++;
    }
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] + memory [0] [6] + memory [0] [7] == 8) {
      memory [0] [random(0,8)] = 0; //if the whole row is 1s, add a 0 somewhere.
      amends ++;
    }
  }
  
/*
There are gonna be some edge cases of rows changing to invalidate columns. 
I'm just gonna trust that there's such a small chance of that occurring
and trust the weaver to visually catch and manually adjust for that.
I'm also noticing it's making seemingly unnecessary amendments, 
but it's called "Glitch Mode" anyway, so it can do what it wants.
Maybe some developer will come along and offer a better system, 
but for now this is good enough.
*/

apply();
}

/////////////////////////////////

void apply() {
//Push to servos// 
  if (memory [0] [0] == 0) {
    s1.write(shedDown);
  } else {
    s1.write(shedUp);
  }
  if (memory [0] [1] == 0) {
    s2.write(shedDown);
  } else {
    s2.write(shedUp);
  }
  if (memory [0] [2] == 0) {
    s3.write(shedDown);
  } else {
    s3.write(shedUp);
  }
  if (memory [0] [3] == 0) {
    s4.write(shedDown);
  } else {
    s4.write(shedUp);
  }
  if (memory [0] [4] == 0) {
    s5.write(shedDown);
  } else {
    s5.write(shedUp);
  }
  if (memory [0] [5] == 0) {
    s6.write(shedDown);
  } else {
    s6.write(shedUp);
  }
  if (memory [0] [6] == 0) {
    s7.write(shedDown);
  } else {
    s7.write(shedUp);
  }
  if (memory [0] [7] == 0) {
    s8.write(shedDown);
  } else {
    s8.write(shedUp);
  }

//Display memory//  
  Serial.print(memory [0] [0]); 
  Serial.print(memory [0] [1]); 
  Serial.print(memory [0] [2]); 
  Serial.print(memory [0] [3]);
  Serial.print(memory [0] [4]);
  Serial.print(memory [0] [5]);
  Serial.print(memory [0] [6]);
  Serial.print(memory [0] [7]);  //display the new top array values
  if (amends > 0) {
      Serial.print(" amendments: ");
      Serial.println(amends);
    } else {
      Serial.println(" ");
    }

//Shift memory//
  for (b = 0; b < 7; b++) { //move memory down a row
    memory [7] [b] = memory [6] [b];
    memory [6] [b] = memory [5] [b];
    memory [5] [b] = memory [4] [b];
    memory [4] [b] = memory [3] [b];
    memory [3] [b] = memory [2] [b];
    memory [2] [b] = memory [1] [b];
    memory [1] [b] = memory [0] [b];
  }
  pick ++;
  prevGlitch = glitch;
  delay(1000); //for making sure someone doesn't accidentally double-tap the pedal. Fast weavers can lower this at their own risk
}