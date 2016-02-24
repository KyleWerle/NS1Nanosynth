/*
 * SIMPLE RANDOMIZED SEQUENCER
 * for Arduino 
 * for Soundmachines NS1 nanosynth
 * 
 *    Kyle Werle
 *      2016-02-24
 * 
 *    PIN 5 - Randomizer button
 *    PIN 6 - Clock input (LFO or analog trigger)
 *    PIN 7 - Sequencer Output
 */

/****SET VARIABLES****/
//  pins on Arduino
const int randomButton = 5;
const int clockPin = 6;
const int seqOut = 7;

//  sequencer array
int sequence[16]; 

//  position of sequencer in array
int seqPos = 0;

//  set state of randomizer button
int randomSet = 0;
int randomStateCurrent = 0;
int randomStatePrevious = 0;

//  to parse clock input
int clockSet = 0;
int clockStateCurrent = 0;
int clockStatePrevious = 0;

/****INIT SETUP****/
void setup() {

  //  set random seed
  randomSeed(analogRead(0)*PI*analogRead(2));

  //  fill sequencer with random data
  randomizeSeq();

  //  set pins on Arduino
  pinMode(randomButton, INPUT);
  pinMode(clockPin, INPUT);
  pinMode(seqOut, OUTPUT);

  //  begin serial for debug
  Serial.begin(9600);
 
}

/****LOOP****/
void loop() {

  //  set state for randomizer button
  randomSet = digitalRead(randomButton);
  //  set state if random button
  if (randomSet == HIGH) {  
    randomStateCurrent = 1;
  } else {
    randomStateCurrent = 0;
  }
  
  //  detect edge of button input
  if (randomStateCurrent != randomStatePrevious) {
    randomStatePrevious = randomStateCurrent;

    //  activate randomizer
    if (randomSet == HIGH) {
      randomizeSeq();
      //  set sequencer back to first position
      seqPos = 0;
      Serial.println("RandomButton");
    }
  }

  //  read clock input
  clockSet = digitalRead(clockPin);
  //  set state of clock
  if (clockSet == HIGH) {
    clockStateCurrent = 1;
  } else {
    clockStateCurrent = 0;
  }

  //  detect edge of clock input
  if (clockStateCurrent != clockStatePrevious) {
    clockStatePrevious = clockStateCurrent;
    
    if (seqPos < 15) {
      seqPos++;
    } else {
      seqPos = 0;
      //Serial.println("SeqPosReset");
    } 

    if (sequence[seqPos] == 1) {
      digitalWrite(seqOut, HIGH); 
    } else {
      digitalWrite(seqOut, LOW);
    }
    
    Serial.print("Position");
    Serial.println(seqPos);
    Serial.print("Output");
    Serial.println(sequence[seqPos]);
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
