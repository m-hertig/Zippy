/*
  Zippy Arduino Code
  Analog serial values, deepest to highest: 169, 1023, 511, 339, 255, 203
*/

byte noteONCommand = 144;//note on command
byte pitchbendCmd = 224;
byte keyPressureCmd = 160;
byte controlChangeCmd = 176;

// number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input. 
const int numReadings = 10;

boolean noteHit[6] = {false,false,false,false,false,false};
byte noteHitNote[6] = {0,0,0,0,0,0};
boolean secondSensorHit[4] = {false,false,false,false};
unsigned long noteHitTime[6] = {0,0,0,0,0,0};
byte midiNotes[6] = {50,55,62,67,71,74};
boolean lastNoteWasHighest = false;
int chordPos[4] = {0,3,5,6};
long noteDuration = 500;
int chordIndex = 0;
int modeNr = 0;
//int hitCounter = 0;
int sensorHitCounter = 0;

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int minReading = 500;                // minimum sensor read
int maxReading = 710;



boolean isPotiOn[4] = {false,false,false,false};
int hitCounter[4] = {0,0,0,0};
int onOffMidiCmd[4] = {63,61,59,65};
int potiMidiCmd[4] = {pitchbendCmd,controlChangeCmd,controlChangeCmd,controlChangeCmd};
int potiMidiChannel[4] = {64,62,60,66};
int lowValCounter = 0;

void setup()
{
  // initialize serial communication with computer:
  // Set MIDI baud rate:
   Serial.begin(31250);  
   // Serial.begin(9600);  
  // initialize all the readings to 0: 
  //  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  //    readings[thisReading] = 0;          
}

void loop() {
  processFirstInput();       
  processSecondInput();
  processThirdInput();
//  processFourthInput();
  delay(10);
}

//send MIDI message
void MIDImessage(byte command, byte data1, byte data2) {
  Serial.write(command);
  Serial.write(data1);
  Serial.write(data2);
}

void processFirstInput() {
  int stringsSensorVal = analogRead(A0);
  //Analog serial values deepest to highest: 169, 1023, 511, 339, 255, 203
  // open g tuning: D-G-D-G-B-D
  // midi notes: D3=50 -> 50-55-62-67-71-74
  if (stringsSensorVal>166) {
    //lastNoteWasHighest = false;  
    if (stringsSensorVal > 166 && stringsSensorVal < 173) {
      hitNote(0);
    } else if (stringsSensorVal > 1015) { 
      hitNote(1);
    } else if (stringsSensorVal > 508 && stringsSensorVal < 514) { 
      hitNote(2);
    } else if (stringsSensorVal > 338 && stringsSensorVal < 342) { 
      hitNote(3);
    } else if (stringsSensorVal > 253 && stringsSensorVal < 258) {  
      hitNote(4); 
    } else if (stringsSensorVal > 200 && stringsSensorVal < 206) { 
      hitNote(5);
      //lastNoteWasHighest = true;    
    }
  }
  for(int i=0;i<6;i++) {
    if(noteHit[i] && abs(millis()-noteHitTime[i])>noteDuration) {
      MIDImessage(noteONCommand, noteHitNote[i], 0);//turn note off
      //Serial.println("Note released");
      noteHitTime[i] = 0;
      noteHit[i] = false;
    }
  }
}

void processSecondInput() {
  int stringsSensorVal = analogRead(A2);
  int newChordIndex = 10;
  //Analog serial values deepest to highest: 169, 1023, 511, 339, 255, 203
  // open g tuning: D-G-D-G-B-D
  // midi notes: D3=50 -> 50-55-62-67-71-74
  if (stringsSensorVal>0) {
    if (stringsSensorVal > 1015) { 
      if (secondSensorHit[0]==false) {
        newChordIndex = 3;
        secondSensorHit[0]=true;
      }
    } else if (stringsSensorVal > 508 && stringsSensorVal < 514) { 
      if (secondSensorHit[1]==false) {
        newChordIndex = 2;
        secondSensorHit[1]=true;
      }
    } else if (stringsSensorVal > 338 && stringsSensorVal < 342) { 
       if (secondSensorHit[2]==false) {
        newChordIndex = 1;
        secondSensorHit[2]=true;
      }
    } else if (stringsSensorVal > 253 && stringsSensorVal < 258) {  
      if (secondSensorHit[3]==false) {
        newChordIndex = 0;
        secondSensorHit[3]=true;
      }
    }
    if (newChordIndex != chordIndex && newChordIndex != 10) {
      chordIndex = newChordIndex;
      for(int i=0;i<6;i++) {
        //turn all notes off
        MIDImessage(noteONCommand, midiNotes[i], 0);
        noteHitTime[i] = 0;
        noteHit[i] = false;
      }
    }
  } else {
    for(int i=0;i<4;i++) {
      secondSensorHit[i]=false;
    }
  }

  
  for(int i=0;i<6;i++) {
    if(noteHit[i] && abs(millis()-noteHitTime[i])>noteDuration) {
      MIDImessage(noteONCommand, midiNotes[i], 0);//turn note off
      //Serial.println("Note released");
      noteHitTime[i] = 0;
      noteHit[i] = false;
    }
  }
}

void hitNote(int index) {
  noteHitTime[index] = millis();
  if (noteHit[index] == false) {
    //Serial.println("Note hit");
    // Regular note+Chordw
    byte noteToHit = midiNotes[index]+chordPos[chordIndex];
    noteHit[index] = true;
    if (noteHitNote[index] != noteToHit) {
      // mute previous note before you play the next
      MIDImessage(noteONCommand, noteHitNote[index], 0);//turn note on
    }
    noteHitNote[index] = noteToHit;
    MIDImessage(noteONCommand, noteToHit, 100);//turn note on
  }
}

void processThirdInput() {
  // smoothing sensor input
  total= total - readings[index];         
  readings[index] = analogRead(A4); 
  total= total + readings[index];       
  index = index + 1;                    
  if (index >= numReadings) {            
    index = 0;                           
  }
  average = total / numReadings;
  
  if (average > minReading && average < maxReading) { 

    byte noteUnconstrained = map(average, minReading, maxReading, 0, 127);//use the 0-900 range I measured
    byte note = constrain(noteUnconstrained,0,127);
    //note = random(0,127);
    //MIDImessage(potiMidiCmd[chordIndex], potiMidiChannel[chordIndex], note);
    // Simplify
    MIDImessage(pitchbendCmd, potiMidiChannel[chordIndex], note);
    delay(10);
    if (isPotiOn[chordIndex]==false) {
      // this one is alwas a control change cmd
      MIDImessage(controlChangeCmd, onOffMidiCmd[chordIndex], 100);
      delay(10);
      isPotiOn[chordIndex]=true;
    }
    lowValCounter=0;
    delay(50);//hold note for 300ms
  } else if (average < minReading) {
    if (lowValCounter<15) {
      lowValCounter++;
    } else if (isPotiOn[chordIndex]==true) {
      //isPotiAlreadyOn=false;
      MIDImessage(potiMidiCmd[chordIndex], potiMidiChannel[chordIndex], 0);
      delay(10);
      MIDImessage(controlChangeCmd, onOffMidiCmd[chordIndex], 0);
      isPotiOn[chordIndex] = false;
      lowValCounter=0;
      delay(5);
    }
  }
  //} 
  //delay(10);        // delay in between reads for stability     
}




