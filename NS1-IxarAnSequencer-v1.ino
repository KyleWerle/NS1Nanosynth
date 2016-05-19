/*
 * IXAR'AN SEQUENCER
 * for Arduino 
 * for Soundmachines NS1 nanosynth
 * for Summoning Dark Lords of Immaterial Planes
 * 
 *    Kyle Werle
 *      2016-02-24
 *    PIN 5 - Gate Randomizer button
 *    PIN 6 - Sequencer Gate Output
 *    PIN 7 - Clock input (LFO or analog trigger)
 *    PIN 8 - Record Control (Ribbon GATE)
 *    PIN 13 - Record Mode LED
 *    ANALOG PIN 1 - Length input (C1)
 *    ANALOG PIN 2 - CV Length input (C2)
 *    ANALOG PIN 3 - CV Input (Ribbon HLD)
 *    
 *    POSSIBLE ADDITIONS********
 *      EEPROM storage of randomSeed to save sequences
 *      extra gate and CV sequencers
 *      use 2 more more buttons
 *      
 *    CHANGE LOG****************
 *      v1
 *      2016-05-17
 *        added CV sequencer with record mode switch
 *        implemented the MCP4451 quad digipot on NS1
 *      2016-5-18
 *        switched gate output and clock input pins
 *        changed record behavior to be controlled by ribbon gate
 */

/****LIBRARIES****/
//  i2c lib to drive the quad digipot
#include "Wire.h"

/****SET VARIABLES****/
//  pins on Arduino
const int randomButton = 5;
const int seqOut = 6;
const int clockPin = 7;
const int recordControl = 8;
const int recordModeLED = 13;
const int lengthIn = A1;
const int CVlengthIn = A2;
const int CVinput = A3;

//  sequencer arrays
int CVsequence[16];
int sequence[16];

//  sequencer lengths
int CVseqL = 15; //0-15
int seqL = 15; //0-15

//  position of sequencer in array
int CVseqPos = 0;
int seqPos = 0;

//  record Mode
bool recordMode = false;

//  read CV input
int CVreader = 0;
//  quantise CV output
unsigned char quantiser = PI;
unsigned char CVquantise;

//  state of recorder button
bool recordState = false;
bool recordStatePrevious = false;

//  state of randomizer button
bool randomState = false;
bool randomStatePrevious = false;

//  parse clock input
bool clockState = false;
bool clockStatePrevious = false;

//  digipot variables
byte addresses[4] = { 0x00, 0x10, 0x60, 0x70 };
byte digipot_addr = 0x2C; //  i2c bus IC addr

/****INIT SETUP****/
void setup() {
  //  begin i2c
  Wire.begin();

  //  set random seed
  randomSeed(analogRead(A5)*PI*analogRead(A4));
  
  //  fill sequencer with random data
  randomizeSeq();

  //  set pins on Arduino
  pinMode(randomButton, INPUT);
  pinMode(seqOut, OUTPUT);
  pinMode(clockPin, INPUT);
  pinMode(recordControl, INPUT);
  pinMode(recordModeLED, OUTPUT);
  pinMode(lengthIn, INPUT);
  pinMode(CVlengthIn, INPUT);
  pinMode(CVinput, INPUT);

  //  begin serial for debug
  //Serial.begin(9600);
 
}

/****LOOP****/
void loop() {
  //  read and map CVlength to 2-16 steps
  CVseqL = map(analogRead(CVlengthIn), 0, 1023, 1, 15);
  
  //  read and map length input to 2-16 steps
  seqL = map(analogRead(lengthIn), 0, 1023, 1, 15);

  //  set state for recorder button
  recordState = digitalRead(recordControl);

  //  detect edge of record button input
  if (recordState != recordStatePrevious) {
    recordStatePrevious = recordState;

    //  set record mode on when gate is held down
    if (recordState == HIGH) {
      recordMode = true;
      
      //  enable LED for record mode
      digitalWrite(recordModeLED, HIGH);
    } else {
      recordMode = false;
      
      //  disable LED
      digitalWrite(recordModeLED, LOW);
    }
  }
  
  //  set state for randomizer button
  randomState = digitalRead(randomButton);

  //  detect edge of random button input
  if (randomState != randomStatePrevious) {
    randomStatePrevious = randomState;

    //  activate randomizer
    if (randomState == HIGH) {
      randomizeSeq();
      //  set sequencer back to first position
      seqPos = 0;
    }
  }

  //  read clock input and set state of clock
  clockState = digitalRead(clockPin);

  //  detect edge of clock input
  if (clockState != clockStatePrevious) {
    clockStatePrevious = clockState;

    //  only progress if clock is HIGH
    if (clockState == HIGH); {
      //  move CV sequencer position forward
      if (CVseqPos < CVseqL) {
        CVseqPos++;
      } else {
      CVseqPos = 0;
      }

      //  read CV input when record mode is active
      if (recordMode) {
        //  read incoming CV input and map to 255
        CVreader = map(analogRead(CVinput), 0, 1023, 0, 254);

        //  quantise CV sequencer to PI
        CVquantise = (int)(quantiser*(float)((int)((float)CVreader/quantiser+0.5)));
        
        //  record CV data into current sequence memory
        CVsequence[CVseqPos] = CVquantise;
      }

      //  write current CV sequence data to digipot
      DigipotWrite(0, CVsequence[CVseqPos]);
      
      //  move gate sequencer position forward
      if (seqPos < seqL) {
        seqPos++;
      } else {
        seqPos = 0;
      } 
  
      //  read sequence data and set pin output
      if (sequence[seqPos] == 1) {
        digitalWrite(seqOut, HIGH); 
      } else {
        digitalWrite(seqOut, LOW);
      }
      
    }
      
  }
  
}

/****FUNCTIONS****/
//  fill sequence array with random values
void randomizeSeq() {

  for (int i = 0; i <= 15; i++) {
    float randomF = random(2);
    int randomC = (int) randomF;
    
    sequence[i] = randomC;
  } 
}

//  write a value on a digipot in the IC
void DigipotWrite(byte pot,byte val) {
  i2c_send( digipot_addr, 0x40, 0xff );
  i2c_send( digipot_addr, 0xA0, 0xff );
  i2c_send( digipot_addr, addresses[pot], val);  
}

//  wrapper for i2c digipot routines
void i2c_send(byte addr, byte a, byte b) {
  Wire.beginTransmission(addr);
  Wire.write(a);
  Wire.write(b);
  Wire.endTransmission();
}
