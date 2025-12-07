/*
Arachne 0.4.2B
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
  created 2005
  by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011
  by Tom Igoe
  This example code is in the public domain.
  https://docs.arduino.cc/built-in-examples/digital/Button/
modified again by me, obviously
*/

const int pedalPin = 8;  // the number of the pushbutton pin
const int toggleUp = 12;
const int toggleDown = 13;  //SophieNote: there's a central "off" position to this switch, too

// variables will change:
int pedalState = 0;  // variable for reading the pushbutton status
int upState = 0;
int downState = 0;

void setup() {
  Serial.begin(9600);
  // initialize the pushbutton pin as an input:
  pinMode(pedalPin, INPUT);
  pinMode(toggleUp, INPUT);
  pinMode(toggleDown, INPUT);
}

void loop() {
  // read the state of the pushbutton value:
  pedalState = digitalRead(pedalPin);
  upState = digitalRead(toggleUp);
  downState = digitalRead(toggleDown);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (upState == HIGH) {
    Serial.print("Glitching,");
  } else {
      if (downState == HIGH) {
      Serial.print("Patterning,");
    } else {
      Serial.print("Off,");
    }
  }
  
  if (pedalState == HIGH) {
    Serial.println(" pedal pushed");
  } else {
    Serial.println(" no pedal push");
  }
}


/*
RESULT: success!
Works just fine, registers all possible combinations of inputs. 
Again, not surprising, but it's nice to know I can write basic code.

This indicates that both of the switches are fully connected and responsive.
*/
