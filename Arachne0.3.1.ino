/*
Arachne 0.3
Sophie Weidmann Nov 19 2025
With help from Max Weidmann

----------------

This code is for a servo-controlled dobby loom I call Arachne, because of both its and my hubris! 
The loom is capable of both leading and following, in that it can treadle for the pattern you give it, or it can act independently.
Currently it's capable of designing a random weave, edited for structural stability. 

This code is for 8 harnesses, but should be fine to pattern out <8 and just leave a servo or two inactive. 
More harnesses will require more servos, more memory, more microcontroller pins, and more amperage, but is theoretically entirely plausible.

----------------

Future plans involve:
Building a library of treadling patterns for glitch mode to riff off of.
Adding a little knob/dial/potentiometer for switching between treadling presets on the fly?
Giving it a little display for pick count, error codes, and a personality!

----------------

Pinouts:
sensor of your choice (I'm using a DHT20 for humidity) connected via I2C (pins 2 and 3 on pro micro)

toggle switch (single pole, triple throw) glitch mode: 4
toggle switch (single pole, triple throw) pattern mode: 5
  leave third throw disconnected as an "off" position

pick advance pedal: 6
servos (in order): 10, 16, 14, 15, 18, 19, 20, 21
    don't blame me, that's sequential on the pro micro. I'm disgruntled by it, too.

----------------

I'm marking emergency problems by commenting below them with exclamation points and no indent, like so:

    code code
    code code code
//!!! something's wrong here

All issues, be they big or little, are marked with >1 exclamation points so you can easily ctl-F find them. 
Emergencies have three exclamation points, smaller problems/optimizations only have two. Please ignore the small problems for now.

I fried my two pro micro chips (probably from a semester of development, not one specific thing) :( 
I'm planning on testing little pieces of this code on an uno, and rewrite this code from the ground up.
*/

#include <Servo.h>
#include <Adafruit_AHTX0.h>

int glitch = 4;
int pattern = 5;
int advance = 6;

int genSeed;  //this is what the dht data turns into, to generate bools from
int reading;
int advState = 0;
int lastAdvState;
int a;  //a and b are for counting and shuffling things around in the matrix. 
int b;
int pick; //weaving terminology, it means "what number row we're on"
bool busy;  //this is purely preventative, so we can't button spam the advance pedal.
            //!! does this do anything? it seems like a good idea, but does it actually help?

Servo s1;
Servo s2;
Servo s3;
Servo s4;
Servo s5;
Servo s6;
Servo s7;
Servo s8;

Adafruit_AHTX0 aht;

bool memory [8] [8];

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

  s1.attach(10);
  s2.attach(16);
  s3.attach(14);
  s4.attach(15);
  s5.attach(18);
  s6.attach(19);
  s7.attach(20);
  s8.attach(21);

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
    s7.write(0);
    s8.write(0);
  }

    else {
    sensors_event_t humidity;
    aht.getEvent(&humidity);// populate temp and humidity objects with fresh data
    genSeed = 100 * humidity.relative_humidity; //turn it into a usable variable
    Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH"); 
    delay(500);       //!! delays are far from ideal, but this and the baud rate are preventing overflow. Figure out a better solution to refresh data only when needed?
//!!! For some reason the test code (adafruit_aht_test) which has this same exact code works fine, but in this code, it just gives 0s. 
//Is it a power problem, where this code is trying to do so many different things whereas that one only needs to power the one thing?
  
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

void tabby() {
  busy = true;
  if (pick % 2 == 0) {
    memory [0] [0] = 1;
    memory [0] [1] = 0;
    memory [0] [2] = 1;
    memory [0] [3] = 0;
    memory [0] [4] = 1;
    memory [0] [5] = 0;
    memory [0] [6] = 1;
    memory [0] [7] = 0;
  }
  else {
    memory [0] [0] = 0;
    memory [0] [1] = 1;
    memory [0] [2] = 0;
    memory [0] [3] = 1;
    memory [0] [4] = 0;
    memory [0] [5] = 1;
    memory [0] [6] = 0;
    memory [0] [7] = 1;  
    //!! turn these into for loops, tighten up the code
  }
  apply();
}

///////////////////////////

void makePattern() {
  busy = true;
//!!! Here's the big struggle. I can't refresh humidity data outside of the loop function (something something local variables??) 
//I think my genSeed solution fixes it, but I'm getting all 0s I think because of a hardware issue, not a software one. 
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
  aht.getEvent(&humidity, &temp);
  if (humidity.relative_humidity % 2 == 0) {
    memory [0] [3] = 0;
  }
  aht.getEvent(&humidity, &temp);
  if (humidity.relative_humidity % 2 == 0) {
    memory [0] [4] = 0;
  }
  aht.getEvent(&humidity, &temp);
  if (humidity.relative_humidity % 2 == 0) {
    memory [0] [5] = 0;
  }
  aht.getEvent(&humidity, &temp);
  if (humidity.relative_humidity % 2 == 0) {
    memory [0] [6] = 0;
  }
  aht.getEvent(&humidity, &temp);
  if (humidity.relative_humidity % 2 == 0) {
    memory [0] [7] = 0;
  }
  */
//!!! Here's the mangled corpse of my original code.
// Theoretically, it's supposed to look at the humidity, see if that ends in an odd or even number, then assign a bool value to the first place in the first array. That repeats for each place in the first array.


  //row weavecheck, to ensure structural validity of the fabric

  if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] + memory [0] [6] + memory [0] [7] == 0) {
    memory [0] [random(0,8)] = 1;
  }
  if (memory [0] [0] + memory [0] [1] + memory [0] [2] + memory [0] [3] + memory [0] [4] + memory [0] [5] + memory [0] [6] + memory [0] [7] == 8) {
    memory [0] [random(0,8)] = 0;
  }

  //column weavecheck, to ensure structural validity of the fabric
  for (a = 0; a < 7; a++) {
  if (memory [0] [a] + memory [1] [a] + memory [2] [a] + memory [3] [a] + memory [4] [a] + memory [5] [a] + memory [6] [a] + memory [7] [a] == 0) {
    memory [0] [a] = 1;
  }
  if (memory [0] [a] + memory [1] [a] + memory [2] [a] + memory [3] [a] + memory [4] [a] + memory [5] [a] + memory [6] [a] + memory [7] [a] == 8) {
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

  //!! eventually, the setting of the dial potentiometer should determine which pattern is selected
  //!! put patterns here

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
  s7.write(memory [0] [6]);
  s8.write(memory [0] [7]);

  //print so you can visually see the 0s and 1s in the console when testing
  Serial.print(memory [0] [0]);
  Serial.print(memory [0] [1]);
  Serial.print(memory [0] [2]);
  Serial.print(memory [0] [3]);
  Serial.print(memory [0] [4]);
  Serial.print(memory [0] [5]);
  Serial.print(memory [0] [6]);
  Serial.println(memory [0] [7]);
  //inelegant, but effective. Leave be for now.

  //move memory down a row
  for (b = 0; b < 7; b++) {
    memory [7] [b] = memory [6] [b];
    memory [6] [b] = memory [5] [b];
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