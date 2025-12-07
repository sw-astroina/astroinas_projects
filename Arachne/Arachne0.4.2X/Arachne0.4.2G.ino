/*
Arachne 0.4.2G
Sophie Weidmann Dec 7 2025

The 0.4.2X versions are little chunks of code, made 
to test specific functionalities of the larger program. 
See 0.4.1 for context.

What are we testing?
  A: controlling 2 servos at once
  B: reading the 3 switch inputs
  C: reading the DHT20 and turning the recieved float into a bool
  D: controlling the two servos based on the DHT's data
  E: holding and updating a 6x6 matrix array's worth of memory
  F: generating bools from the DHT20 to be stored in the memory
G: generating those bools when the switches are in correct config
  H: applying those bools to the servos (I currently only have two)

Evidence of my swashbucklery and brigandage:
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

Adafruit_AHTX0 aht;

//Pin Declarations//
const int pedalPin = 8;  
const int toggleUp = 12;
const int toggleDown = 13;  //there's a central "off" position to this toggle switch, too

//Memory Variables//
bool memory [6] [6]; //INFO: we need to remember what we've recently woven to prevent floats. 
                    //Not number floats, but threads that float above the fabric and compromise the structural stability.
                    //Essentially we're just checking that it's not all 0s or all 1s in any given row or column.

bool columnChange;  //these are for shuffling things around in the matrix. 
int changedColumn;
int changingRow;
int a;  
int b;  //Max named a and b, and I'm too lazy to rename for clarity.

//Switches// 
bool prevPedalState = LOW; 
bool pedalState = LOW;  //these are for the status of the switches
bool upState = LOW;
bool downState = LOW;

//Miscellaneous//
bool on;         //self-explanatory. Is the switch set to "off"?
bool glitch;     //if the loom should generate its own pattern, glitch is true
bool prevGlitch = false; //we're tracking this so that we can check pattern validity when transitioning out of glitch mode

int genSeed;     //this is what the dht humidity data turns into, to generate bools from
int pick = 0;        //pick count is tracking how many rows we've woven. It's essentially "how many times have we done this?"
int amends = 0;   //how many times has the pattern been changed when weavechecking? Just for display, to see what's happening under the hood

/////////////////////////////////

void setup() {
  Serial.begin(9600);

  pinMode(pedalPin, INPUT); //declare switch pins
  pinMode(toggleUp, INPUT);
  pinMode(toggleDown, INPUT);

  if (! aht.begin()) {
    Serial.println("Could not find DHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("DHT20 found");
}

/////////////////////////////////

void loop() {
  pedalState = digitalRead(pedalPin); //read the sensors
  upState = digitalRead(toggleUp);
  downState = digitalRead(toggleDown);

  // Serial.print(pedalState);
  // Serial.print(", previously ");
  // Serial.println(prevPedalState);  //for debugging

//Detect falling edge of pedal//
  if (pedalState == LOW && prevPedalState == HIGH) {  //the pedal has a pull down resistor, so it shouldn't need debouncing?
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
      }
    }


    if (on == true) {
      if (glitch == false) {
//Follow pattern//
        if (pick % 2 == 0) {
          memory [0] [0] = 1;
          memory [0] [1] = 0;
          memory [0] [2] = 1;
          memory [0] [3] = 0;
          memory [0] [4] = 1;
          memory [0] [5] = 0;
        } else {
          memory [0] [0] = 0;
          memory [0] [1] = 1;
          memory [0] [2] = 0;
          memory [0] [3] = 1;
          memory [0] [4] = 0;
          memory [0] [5] = 1;
        }
        if (prevGlitch == true) {
          weavecheck();
        }
        else {
          apply(); //skip weavecheck, under assumption that weaver wants what they want regardless of structural validity
        }


      } else {
//Generate new row//
        for (a = 0; a < 6; a++) {
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
    }
  }
  prevPedalState = pedalState; // Update previous pedal state for next loop
}

/////////////////////////////////

void weavecheck() {
  amends = 0;
//Check warp/column//
  columnChange = false;
  for (a = 0; a < 5; a++) {
    if (memory [0] [a] == memory [1] [a]) {
      if (memory [0] [a] == memory [2] [a]) {
        if (memory [0] [a] == memory [3] [a]) {
          if (memory [0] [a] == memory [4] [a]) {
            if (memory [0] [a] == memory [5] [a]) {
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
  } //are nesting ifs bad practice? I think this one's readable, but then again I'm the one who wrote it

//Check weft/row//
  if (columnChange == true) {
    changingRow = random(0,6);
    if (changingRow == changedColumn) {
      changingRow++;
    }
    if (changingRow > 5) {
      changingRow = 0;   //if a column was changed, and the row still needs to be changed (1.2% chance), 
                         //don't change the rightmost column that changed. Imperfect system...
    } 
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 0) {
      memory [0] [changingRow] = 1; //if the whole row is 0s, change the 0 to the right of the last changed column to be a 1.
      amends ++;
    }
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 6) {
      memory [0] [changingRow] = 0; //if the whole row is 1s, change the 0 to the right of the last changed column to be a 1.
      amends ++;
    }
  } else {
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 0) {
      memory [0] [random(0,6)] = 1; //if the whole row is 0s, add a 1 somewhere.
      amends ++;
    }
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 6) {
      memory [0] [random(0,6)] = 0; //if the whole row is 1s, add a 0 somewhere.
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


void apply() {
//Display and update memory//  
  Serial.print(memory [0] [0]); 
  Serial.print(memory [0] [1]); 
  Serial.print(memory [0] [2]); 
  Serial.print(memory [0] [3]);
  Serial.print(memory [0] [4]);
  Serial.print(memory [0] [5]);  //display the new top array values

  for (b = 0; b < 5; b++) { //move memory down a row
    memory [5] [b] = memory [4] [b];
    memory [4] [b] = memory [3] [b];
    memory [3] [b] = memory [2] [b];
    memory [2] [b] = memory [1] [b];
    memory [1] [b] = memory [0] [b];
  }

  if (amends > 0) {
    Serial.print(" amendments: ");
    Serial.println(amends);
  } else {
    Serial.println(" ");
  }

  pick ++;
  prevGlitch = glitch;
  delay(800); //for making sure someone doesn't accidentally double-tap the pedal. 
}              //Fast weavers can lower this at their own risk, but do not take below 50.


/*
cut the delay (add busy variable?), add patterns
RESULT: HA! Finally, it works! Thank you Kreg!
*/
