/*
Arachne 0.4.2F
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
F: generating bools from the DHT20 to be stored in the memory
  G: generating those bools when the switches are in correct config
  H: applying those bools to the servos (I currently only have two)

Evidence of my swashbucklery and brigandage:
  No ReadMe in code, but stolen from 
  adafruit_aht_test in the AdafruitAHTX0 library

  For loops were stolen from texts from Max Weidmann
  Mwahahaha stealing from siblings >:]
modified by me, obviously
*/

#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
int genSeed;  //this is what the dht humidity data turns into, to generate bools from

bool memory [6] [6]; //INFO: we need to remember what we've recently woven to prevent floats. 
                    //Not number floats, but threads that float above the fabric and compromise the structural stability.
                    //Essentially we're just checking that it's not all 0s or all 1s in any given row or column.

bool columnChange;
int changedColumn;
int changingRow;
int a;  //these are for shuffling things around in the matrix. 
int b;  //Max named a and b, and I'm too lazy to rename for clarity.

void setup() {
  Serial.begin(9600);
  if (! aht.begin()) {
    Serial.println("Could not find DHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("DHT20 found");
}


void loop() {
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

  //warp/column checker, to ensure structural validity of the fabric 
  columnChange = false;
  for (a = 0; a < 5; a++) {
    if (memory [0] [a] == memory [1] [a]) {
      if (memory [0] [a] == memory [2] [a]) {
        if (memory [0] [a] == memory [3] [a]) {
          if (memory [0] [a] == memory [4] [a]) {
            if (memory [0] [a] == memory [5] [a]) {
              if (memory [0] [a] == 0) {
                memory [0] [a] = 1; //if the whole column is 0s, change the top digit to be a 1.
              }
              else {
                memory [0] [a] = 0; //if the whole column is 1s, change the top digit to be a 0.
              }
              columnChange = true;
              changedColumn = a; // I can only mark one column as being changed at a time. How to fix?
            }
          }
        }
      }
    }
  } //are nesting ifs bad practice? I think this one's readable, but then again I'm the one who wrote it


  //weft/row checker, to ensure structural validity of the fabric
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
    }
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 6) {
      memory [0] [changingRow] = 0; //if the whole row is 1s, change the 0 to the right of the last changed column to be a 1.
    }
  } else {
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 0) {
      memory [0] [random(0,6)] = 1; //if the whole row is 0s, add a 1 somewhere.
    }
    if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 6) {
      memory [0] [random(0,6)] = 0; //if the whole row is 1s, add a 0 somewhere.
    }
  }

/*
There are gonna be some edge cases of rows changing to invalidate columns. 
I'm just gonna trust that there's such a small chance of that occurring
and trust the weaver to visually catch and manually adjust for that.
Maybe some developer will come along and offer a better system, 
but for now this is good enough.
*/

  
  Serial.print(memory [0] [0]); 
  Serial.print(memory [0] [1]); 
  Serial.print(memory [0] [2]); 
  Serial.print(memory [0] [3]);
  Serial.print(memory [0] [4]);
  Serial.println(memory [0] [5]);  //display the new top array values


  for (b = 0; b < 5; b++) { //move memory down a row
    memory [5] [b] = memory [4] [b];
    memory [4] [b] = memory [3] [b];
    memory [3] [b] = memory [2] [b];
    memory [2] [b] = memory [1] [b];
    memory [1] [b] = memory [0] [b];
  }


  delay(2000); //entirely arbitrary, for readability of serial monitor
}


/*
RESULT: success!

Combining C and E went well, no issues yet.
*/