/*
Arachne 0.4.1
Sophie Weidmann Dec 3 2025
With help from Max Weidmann

----------------

This code is for a servo-controlled dobby loom I call Arachne, because of both its and my hubris! 
The loom is capable of both leading and following, in that it can lift the shed for the pattern you give it, or it can act independently.
Currently it's capable of designing a random weave structue, edited for fabric stability. 
This code is for 6 harnesses, but should be fine to test two for now and expand to 6 later.

----------------

Pinouts:
DHT20 connected via I2C

toggle switch (single pole, triple throw) glitch mode: 13
toggle switch (single pole, triple throw) pattern mode: 12
  leave third throw disconnected as an "off" position

pick advance pedal: 8
servos (in order): 3, 5, 6, 9, 10, 11
                //somebody needs to make a board with sequential PWM pins 

----------------

Information on how looms work and what the code does to operate the loom has been sprinkled in under the assumption that
the venn diagram of people who read this and people who weave isn't a broad overlap. These notes are labelled in all caps:

//INFO: No idea what I'm taking about? see https://en.wikipedia.org/wiki/Dobby_loom

----------------

I'm marking emergency problems by commenting below them with exclamation points and no indent, like so:

    code code code
//!!! something's wrong here

All issues are marked with >1 exclamation points so you can easily ctrl-F find them. 
Emergencies have three exclamation points, smaller problems/optimizations only have two.

----------------

Future plans involve:
Building a library of common shed patterns (plain, basket, S twill, Z twill, chevron, diaper, waffle, satin variants (5/1, 4/2, 2/4, 1/5))
Adding a clicking potentiometer for switching between patterns (low torque, 12 detents. Leave last detent for custom pattern)
Giving it a display for pick count, pattern name, and error codes (build in error codes)

Expand to control more harnesses (Communicate to multiple servos through 1 pin. Servo driver daughter board?)
Improving glitch mode (make a little baby AI to look for and elaborate on patterns in its memory)
*/

#include <Servo.h>
#include <Adafruit_AHTX0.h>

Servo s1;
Servo s2;
Servo s3;
Servo s4; //INFO: each servo lifts a harness of threads in the loom.
Servo s5; //It pulls some of the warp threads up, so we can throw a weft thread between the two sections of warp threads.
Servo s6; //The division of some threads being up and some being down is called the shed.

Adafruit_AHTX0 aht;

int glitch = 13;
int pattern = 12;
int advance = 8;

int reading;
int advState = 0;
int lastAdvState;

int genSeed;  //this is what the dht data turns into, to generate bools from
bool busy;  //this is purely preventative, so we can't button spam the advance pedal.
            //!! does this do anything? it seems like a good idea, but does it actually help?

int pick; //INFO: pick count is tracking how many rows we've woven. It's essentially "how many times have we done this?"
bool memory [6] [6]; //INFO: we need to remember what we've recently woven to prevent floats. 
                    //Not number floats, but threads that float above the fabric and compromise the structural stability.
                    //Essentially we're just checking that it's not all 0s or all 1s in any given row or column.
int a;
int b;  //a and b are for counting and shuffling things around in the matrix. 

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
  bool genMode = digitalRead(glitch);
  bool on = digitalRead(pattern);

///////////////////////////

void setup() {
  Serial.begin(9600);
  pinMode(advance, INPUT);
  pinMode(glitch, INPUT);
  pinMode(pattern, INPUT);
  lastAdvState = 0;
  pick = 0;

  s1.attach(3);
  s2.attach(5);
  s3.attach(6);
  s4.attach(9);
  s5.attach(10);
  s6.attach(11);

  Serial.println("Ready!");
  busy = false;
}

///////////////////////////

void loop() {

  genMode = digitalRead(glitch);
  on = digitalRead(pattern);


  if ((genMode == 0) && (on == 0)) { //if the toggle switch is set to OFF:
    // set all servos to 0 and do nothing else.
    s1.write(0);
    s2.write(0);
    s3.write(0);
    s4.write(0);
    s5.write(0);
    s6.write(0);
  }

    else {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
    genSeed = 100 * humidity.relative_humidity; //turn it into a usable variable
    Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH"); 
    delay(500);       //!! delays are far from ideal, but this and the baud rate are preventing overflow. Figure out a better solution to refresh data only when needed?
//!!! For some reason the test code (adafruit_aht_test) which has this same exact code works fine, but in this code, it just gives 0s. 
//Is it a power problem, where this code is trying to do so many different things whereas that one only needs to power the one thing?
//I've supplied more amperage, but haven't tested the code with more amps yet.
//If it still doesn't work, cut this whole code into pieces and test the pieces individually.
  
    reading = digitalRead(advance);

    // If the advance pedal reading changed due to noise or pressing:
    if (reading != lastAdvState) {
    // reset the debouncing timer
      lastDebounceTime = millis();
    }

    //if the pedal reading's been consistent for longer than the debounce delay, and the loom's not busy thinking, actually read it
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (busy == false) {
        if (reading != advState) {
          if (advState == HIGH) {        //!! is there a way to combine all these ifs so they don't have to nest?

            if (pick < 11) {
              tabby();    
              //for structural stability and to solve the problem of the weavechecker panicking at a blank memory
            }
          // check if it's in glitch mode:
            if (genMode == 1) {
              //glitch mode is on, so generate a random weave structure.
              makePattern();
            } 
            if (on == 1){
              //glitch mode is off, so follow the pre-programmed pattern.
              //followOrders(); 

              //I have no patterns at the moment, so for now we'll just call
              tabby();
              //to simulate a plain weave pattern. Not exciting, but functional for now
            }

          }
          advState = reading;
        }
      }
    }
    lastAdvState = reading;
   }
}

///////////////////////////

void tabby() { //INFO: tabby is the term for a border of really strong fabric,
              //to protect the prettier fabric inside the border from fraying and unraveling.
              //Tabby most often looks like a checkerboard of 0s and 1s, called a plain weave.
  busy = true;
  if (pick % 2 == 0) {
    memory [0] [0] = 1;
    memory [0] [1] = 0;
    memory [0] [2] = 1;
    memory [0] [3] = 0;
    memory [0] [4] = 1;
    memory [0] [5] = 0;
  }
  else {
    memory [0] [0] = 0;
    memory [0] [1] = 1;
    memory [0] [2] = 0;
    memory [0] [3] = 1;
    memory [0] [4] = 0;
    memory [0] [5] = 1; 
    //!! turn these into for loops, tighten up the code
  }
  apply();
}

///////////////////////////

void makePattern() {
  busy = true;
//!!! Here's the big struggle. I can't refresh humidity data outside of the loop function (something something local variables??) 
//I think my genSeed solution fixes it, but I'm getting all 0s (see line 6).
//Is there a way I can either use aht.getEvent 8 times when requested by makePattern, OR request it once, then use that number to seed 8 random bools????
  if (genSeed % 2 == 0) { // You can't % floats, apparently, so genSeed turns humidity.relative_humidity into an int
    memory [0] [0] = 0;
  }
  else { 
    memory [0] [0] = 1;
  }
  /*
    aht.getEvent(&humidity, &temp);// recall to generate new data for each digit in row 0 of the array
  if (humidity.relative_humidity % 2 == 0) {
    memory [0] [1] = 0;
  }
    aht.getEvent(&humidity, &temp);
  if (humidity.relative_humidity % 2 == 0) {
    memory [0] [2] = 0;
  }
//!! Here's the mangled corpse of my original code. It continues for the rest of the array.
// Theoretically, it's supposed to look at the humidity, see if that ends in an odd or even number, 
// then assign a bool value to the first place in the first array. That repeats for each place in the first array.
  */


  //row weavecheck, to ensure structural validity of the fabric
  //INFO: this prevents weft floats

  if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 0) {
    memory [0] [random(0,6)] = 1;
  }
  if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] == 6) {
    memory [0] [random(0,6)] = 0;
  }

  //column weavecheck, to ensure structural validity of the fabric
  //INFO: this prevents warp floats
  for (a = 0; a < 5; a++) {
  if (memory [0] [a] + memory [1] [a] + memory [2] [a] + memory [3] [a] + memory [4] [a] + memory [5] [a] == 0) {
    memory [0] [a] = 1;
  }
  if (memory [0] [a] + memory [1] [a] + memory [2] [a] + memory [3] [a] + memory [4] [a] + memory [5] [a] == 6) {
    memory [0] [a] = 0;
  }
}
  //!! Make sure the column checker changing something doesn't make the row invalid. The chance of this happening is like 1.2%, but it should still get fixed
  // Repeat row and column checks until it passes two in a row?

  //!! Make these more efficient later by stopping looking (break or continue? No, something else) once it sees a difference instead of running through the whole thing even if the second digit was different

  apply();
}

///////////////////////////

void followOrders() {
  busy = true;
/*
  //!! eventually, the setting of the dial potentiometer should determine which pattern is selected
  //!! write code to achieve the patterns below

  //INFO: here's some pattern samples for reference. These are edited for 6 harnesses and to the nearest repeat.
  
plain        basket       twill (S)    satin (5/1)
010101       001100       110011       100000
101010       001100       011001       000100
             110011       001100       010000
             110011       100110       000001
                                       001000                
                                       000010

//INFO: Weave structures (how the fabric looks) aren't necessarily synonymous with shed patterns (how the servos move). 
//We can only control the shed with a dobby loom like this. However, if we thread the loom like 
//1A, 2B, 3C, 4D, 5E, 6F, 7A, etc, we can get a simple repeat with no extra math required!
//math is of course always an option, if I want to do tieup matrix sudoku later and rethread everything
*/
  apply();
}

///////////////////////////

void apply() {
  //apply row 0 of matrix to servos
  s1.write(memory [0] [0]);
  s2.write(memory [0] [1]);
  s3.write(memory [0] [2]);
  s4.write(memory [0] [3]);
  s5.write(memory [0] [4]);
  s6.write(memory [0] [5]);

  //print so you can visually see the 0s and 1s in the console when testing
  Serial.print(memory [0] [0]);
  Serial.print(memory [0] [1]);
  Serial.print(memory [0] [2]);
  Serial.print(memory [0] [3]);
  Serial.print(memory [0] [4]);
  Serial.println(memory [0] [5]);
  //inelegant, but effective. Leave be for now.

  //move memory down a row
  for (b = 0; b < 5; b++) {
    memory [5] [b] = memory [4] [b];
    memory [4] [b] = memory [3] [b];
    memory [3] [b] = memory [2] [b];
    memory [2] [b] = memory [1] [b];
    memory [1] [b] = memory [0] [b];
  }

  pick++;   
  advState = reading;
  lastAdvState = reading;
  busy = false;
}