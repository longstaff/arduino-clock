#include <Wire.h>
#define DS1307_ADDRESS 0x68
const byte zero = 0x00;

//Current time/state
int second = 0;
int minute = 0;
int hour = 0;
int weekDay = 0;
int monthDay = 0;
int month = 0;
int year = 0;

//Update status
int currentHour = -1;

//For input
int Position, Press;
int isForward = 0;
int rotationTick = 0;
int rotationDirection = 0;
const int rotationTickLimit = 3000;

//For update states
enum updateState
{
  // Rotary encoder input 
  updateStateInit=0,
  updateStateClock=1,
  updateStateHours=2,
  updateStateMins=3,
};

updateState currentState = updateStateMins;
int flashTick = 0;
const int flashTickBlack = 1000;
const int flashTickWhite = 5000;
bool needsDisplayUpdate = true;
  

const int breakpointLength = 6;
const int breakPoints[breakpointLength] = {
  0,
  5,
  12,
  13,
  18,
  22
};
const String breakPointsVals[breakpointLength] = {
  "NIGHT",
  "MORNING",
  "NOON",
  "AFTERNOON",
  "EVENING",
  "NIGHT"
};

/*
//DEMO FOR HOURS
const int breakpointLength = 24;
const int breakPoints[24] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
};
const String breakPointsVals[24] = {
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23"
};
*/

// Digital pin definitions for rotary
enum enDigitalPins
{
  // Rotary encoder input lines
  dpInEncoderA=8,
  dpInEncoderB=7,
  dpInEncoderPress=6,
};

//For the output display
const int latchPin = 10;
const int clockPin = 11;
const int dataPin = 12;

//Constant for number of output LEDs
const int charPrintNumber = 9;

const int charCount = 24;
const char charArray[charCount] = {
  ' ',
  '*',
  'A',
  'O',
  'R',
  'M',
  'N',
  'I',
  'G',
  'F',
  'T',
  'E',
  'V',
  'H',
  '0',
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9'
};
const byte topArray[charCount] = {
  0,
  255,
  220,
  92,
  220,
  58,
  56,
  69,
  84,
  212,
  69,
  212,
  18,
  152,
  92,
  
  8,
  204,
  76,
  152,
  212,
  212,
  76,
  220,
  220
};
const byte bottomArray[charCount] = {
  0,
  255,
  152,
  92,
  168,
  24,
  56,
  69,
  220,
  8,
  1,
  76,
  10,
  152,
  92,
  
  16,
  204,
  212,
  144,
  212,
  220,
  16,
  220,
  144
};

static void _ResetRotaryPins()
{
  // Rotary encoder input lines
  // Configure as input, turn on pullup resistors
  pinMode(dpInEncoderA, INPUT);
  digitalWrite(dpInEncoderA, HIGH);
  pinMode(dpInEncoderB, INPUT);
  digitalWrite(dpInEncoderB, HIGH);
  pinMode(dpInEncoderPress, INPUT);
  digitalWrite(dpInEncoderPress, HIGH);
}

void _lowlevel_ReadRotaryEncoder(int &rotate, int& press)
{
  rotate = (digitalRead(dpInEncoderB) * 2) + digitalRead(dpInEncoderA);
  press = digitalRead(dpInEncoderPress);
}

void ReadRotaryEncoder()
{  
  int Position2, Press2;
  _lowlevel_ReadRotaryEncoder(Position2, Press2);
  
  if(rotationTick > rotationTickLimit){
    rotationTick = 0;
  }
  
  if(currentState != updateStateClock){
    if (Position2 != Position)
    {
      // "Forward" is shown by the position going from (0 to 1) or (1 to 3)
      // or (3 to 2) or (2 to 0).  Anything else indicates that the user is
      // turning the device the other way.  Remember: this is Gray code, not
      // binary.
      int isFwd = ((Position == 0) && (Position2 == 1)) ||
                  ((Position == 1) && (Position2 == 3)) ||
                  ((Position == 3) && (Position2 == 2)) ||
                  ((Position == 2) && (Position2 == 0));
      
      if(isFwd){
        rotationDirection ++;
      }
      else{
        rotationDirection --;
      }
      //Serial.println(isFwd ? "FWD" : "BWD");
    }
  }
  
  if(rotationTick == 0){
    if(rotationDirection > 0){
      switch(currentState){
        case updateStateHours:
          hour ++; 
          if(hour > 23){
            hour = 0;
          }
          break;
        case updateStateMins:
          minute ++;
          if(minute > 59){
            minute = 0;
          }
          break;
      }
      needsDisplayUpdate = true;
    }
    else if(rotationDirection < 0){
      switch(currentState){
        case updateStateHours:
          hour --; 
          if(hour < 0){
            hour = 23;
          }
          break;
        case updateStateMins:
          minute --;
          if(minute < 0){
            minute = 59;
          }
          break;
      }
      needsDisplayUpdate = true;
    }
    
    rotationDirection = 0;
  }
      
  rotationTick ++;
  
  if (Press2 != Press)
  {
    //On release
    if(!Press){
      flashTick = 0;
      needsDisplayUpdate = true;
      switch(currentState){
        case updateStateInit:
          currentState = updateStateClock;
          break;
        case updateStateClock:
          currentState = updateStateHours;
          break;
        case updateStateHours:
          currentState = updateStateMins;
          break;
        case updateStateMins:
          setDateTime();
          currentState = updateStateClock;
          break;
      }
    }
    //Serial.println(Press ? "Press" : "Release");
  }
  Position = Position2;
  Press = Press2;
}

byte bcdToDec(byte val)  {
  // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}
byte decToBcd(byte val){
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

void ReadClock(){
  Wire.beginTransmission(DS1307_ADDRESS);
  
  Wire.write(zero);
  Wire.endTransmission();
  
  Wire.requestFrom(DS1307_ADDRESS, 7);
  
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  monthDay = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
     
   if(currentHour != hour){
     needsDisplayUpdate = true;
   }
   
   if(needsDisplayUpdate){
     updateText(hour);
   }
   currentHour = hour;
}
void updateText(int h){
  for(int i = 1; i< breakpointLength; i++){
    if(h<breakPoints[i]){
      printText(breakPointsVals[i-1]);
      return;
    }
  }
  printText(breakPointsVals[breakpointLength-1]);
}

void ShowHours(){
  if(flashTick > flashTickWhite + flashTickBlack){
    flashTick = 0;
    needsDisplayUpdate = true;
  }
  
  String output = "H ";
  if(flashTick+1 > flashTickWhite){
    if(flashTick == flashTickWhite){
      needsDisplayUpdate = true;
    }
  }
  else{
    output.concat(hour);
  }
  
  if(needsDisplayUpdate){
    printText(output);
  }
  
  flashTick = flashTick + 1;
}
void ShowMins(){
  if(flashTick > flashTickWhite + flashTickBlack){
    flashTick = 0;
    needsDisplayUpdate = true;
  }
  
  String output = "M ";
  if(flashTick+1 > flashTickWhite){
    if(flashTick == flashTickWhite){
      needsDisplayUpdate = true;
    }
  }
  else{
    output.concat(minute);
  }
  
  if(needsDisplayUpdate){
    printText(output);
  }
  
  flashTick = flashTick + 1;
}

void printText(String text){
    
  digitalWrite(latchPin, LOW);
    
  for(int i=0; i<charPrintNumber; i++){
    
    //Print character or clear space otherwise
    int wantedpos = 0;
    if(text.length() > i){
      char input = text[i];
      for (int j=0; j<charCount; j++) {
         if (input == charArray[j]) {
           wantedpos = j;
                 
           break;
         }
      }
    }
      
    shiftOut(dataPin, clockPin, MSBFIRST, topArray[wantedpos]);
    shiftOut(dataPin, clockPin, MSBFIRST, bottomArray[wantedpos]);
    
  }
   
  digitalWrite(latchPin, HIGH);
  
    
  needsDisplayUpdate = false;
}

void setDateTime(){
  
  byte newSecond =      0; //0-59
  byte newMinute =      minute; //0-59
  byte newHour =        hour; //0-23
  byte newWeekDay =     weekDay; //1-7
  byte newMonthDay =    monthDay; //1-31
  byte newMonth =       month; //1-12
  byte newYear  =       year; //0-99

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(newSecond));
  Wire.write(decToBcd(newMinute));
  Wire.write(decToBcd(newHour));
  Wire.write(decToBcd(newWeekDay));
  Wire.write(decToBcd(newMonthDay));
  Wire.write(decToBcd(newMonth));
  Wire.write(decToBcd(newYear));

  Wire.write(zero); //start 

  Wire.endTransmission();
}

void setup() {
  
  Wire.begin();
  Serial.begin(9600);
  
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  
  ReadClock();
  
  _ResetRotaryPins();
}

void loop() {
  switch(currentState){
    case updateStateClock:
      ReadClock();
      break;
    case updateStateHours:
      ShowHours();
      break;
    case updateStateMins:
      ShowMins();
      break;
  }
  
  ReadRotaryEncoder();
}
