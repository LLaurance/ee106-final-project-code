#include "play.h"

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define DIR 6
#define PUL 8
#define ENA 37
#define motorInterfaceType 1
long stepsPerMM = 400L; 
int keyLen = 22.5;
int handArr[3] = {0, -keyLen*stepsPerMM, -2*keyLen*stepsPerMM}; //index 0=left servo's offset
int handPins[3] = {0,2,4}; //index 0= left servo's pin, etc
int numWhite = 27;                  
long currPos = 0L;
int currNote = 1;
long number =0 ;

// Create a new instance of the AccelStepper class:

#include <Arduino.h>
#define USE_PCA9685_SERVO_EXPANDER    // Activating this enables the use of the PCA9685 I2C expander chip/board.
#define DISABLE_COMPLEX_FUNCTIONS     // Activating this disables the SINE, CIRCULAR, BACK, ELASTIC, BOUNCE and PRECISION easings. Saves up to 1850 bytes program memory.
#define MAX_EASING_SERVOS 3
#define ENABLE_EASE_CUBIC
#include "ServoEasing.hpp"
#include "PinDefinitionsAndMore.h"

#if defined(USE_PCA9685_SERVO_EXPANDER)
ServoEasing Servo1(PCA9685_DEFAULT_ADDRESS); // If you use more than one PCA9685 you probably must modify MAX_EASING_SERVOS
ServoEasing Servo2(PCA9685_DEFAULT_ADDRESS); // If you use more than one PCA9685 you probably must modify MAX_EASING_SERVOS
ServoEasing Servo3(PCA9685_DEFAULT_ADDRESS); // If you use more than one PCA9685 you probably must modify MAX_EASING_SERVOS
#else
ServoEasing Servo1;
ServoEasing Servo2;
ServoEasing Servo3;
#endif

#define START_DEGREE_VALUE  0 // The degree value written to the servo at time of attach.

dist_path shortestPath(int *noteArr, int startIdx, int endIdx, long motorPos) {
  if (startIdx == endIdx) {                                                                       //recursive base case 
    dist_path d;
    d.dist = 0L;
    d.path = 0L;
    return d;
  }
  long newAbsolutePositionIfweUseServo1 = getMoveTo(noteArr[startIdx], 0);                        //get position to move if servo 0 is used
  long distMovedFromcurrentSpotofCart1 = abs(motorPos - newAbsolutePositionIfweUseServo1);        //get for motor1
  dist_path d1 = shortestPath(noteArr, startIdx+1, endIdx, newAbsolutePositionIfweUseServo1);     
  long totalDistSupposingnewAbsolutePosition1 = distMovedFromcurrentSpotofCart1 + d1.dist;
  
  long newAbsolutePositionIfweUseServo2 = getMoveTo(noteArr[startIdx], 1);                        //get position to move if servo 1 is used
  long distMovedFromcurrentSpotofCart2 = abs(motorPos - newAbsolutePositionIfweUseServo2);        //get for motor1
  dist_path d2 = shortestPath(noteArr, startIdx+1, endIdx, newAbsolutePositionIfweUseServo2);
  long totalDistSupposingnewAbsolutePosition2 = distMovedFromcurrentSpotofCart2 + d2.dist;
  //get for motor 2
  //recurse

  long newAbsolutePositionIfweUseServo3 = getMoveTo(noteArr[startIdx], 2);                //get position to move if servo 2 is used
  long distMovedFromcurrentSpotofCart3 = abs(motorPos - newAbsolutePositionIfweUseServo3);//get for motor1
  dist_path d3 = shortestPath(noteArr, startIdx+1, endIdx, newAbsolutePositionIfweUseServo3);
  long totalDistSupposingnewAbsolutePosition3 = distMovedFromcurrentSpotofCart3 + d3.dist;
  
  //get for 3rd
  //recurse

  if (totalDistSupposingnewAbsolutePosition3<totalDistSupposingnewAbsolutePosition1 && totalDistSupposingnewAbsolutePosition3<totalDistSupposingnewAbsolutePosition2) {
    dist_path d;
    d.dist = totalDistSupposingnewAbsolutePosition3;
    d.path = 10*d3.path + 3;
    return d;
  }

  else if (totalDistSupposingnewAbsolutePosition2<totalDistSupposingnewAbsolutePosition1 && totalDistSupposingnewAbsolutePosition2<totalDistSupposingnewAbsolutePosition3) {
    dist_path d;
    d.dist = totalDistSupposingnewAbsolutePosition2;
    d.path = 10*d2.path + 2;
    return d;
  }
  else {
    dist_path d;
    d.dist = totalDistSupposingnewAbsolutePosition1;
    d.path = 10*d1.path + 1;
    return d;
  }

}

long newShortPath(int *noteArr, int arrLen) {
  long path = 0;
  long dist = 0;
  long l = 0;
  for (int i = 0; i<arrLen; i++) {
    if (getMoveTo(noteArr[i],0) == 0) {
        path = 10 * path + 1;
    }
    else if (getMoveTo(noteArr[i],1) == 0) {
        path = 10 * path + 2;
    }
    else if (getMoveTo(noteArr[i],2) == 0) {
        path = 10 * path + 3;
    }
    else if (noteArr[i]>currNote) {
        l = getMoveTo(noteArr[i], 2);
        dist = dist + l;
        path = 10* path+3;
    }
    else if (noteArr[i]<currNote) {
        l = getMoveTo(noteArr[i], 0);
        dist = dist + l;
        path = 10* path+1;
    }
    currPos = currPos+l;
    currNote = noteArr[i];
  }
  while (path != 0) {
    number = number * 10;
    number = number + path % 10;
    path = path / 10;
  }
  currPos = -2L*keyLen*stepsPerMM;
  path = number;
  return path;
}
 
long getMoveTo(int note, int handIdx) {
  long moveto = 0;
  long motor_offset = handArr[handIdx];
  if (note == 0) {
    moveto = -1;
  }
  else if (note<=35) {
    moveto = (keyLen*(note-1))*stepsPerMM + motor_offset - currPos;
  }
  return moveto;
}


/*dist_path newShortPath(int *noteArr, int arrLen) {
  dist_path d1;
  d1.dist = 0;
  d1.path = 0;
  long l = 0;
  for (int i = 0; i<arrLen; i++) {
    if (getMoveTo(noteArr[i],0) == 0) {
        long x = 10L * d1.path + 1L;
        d1.path = x;
    }
    else if (getMoveTo(noteArr[i],1) == 0) {
        long x = 10L * d1.path + 2L;
        d1.path = x;
    }
    else if (getMoveTo(noteArr[i],2) == 0) {
        long x = 10L * d1.path + 3L;
        d1.path = x;
    }
    else if (noteArr[i]>currNote) {
        long l = getMoveTo(noteArr[i], 2);
        d1.dist = d1.dist + l;
        long x = 10L * d1.path + 3L;
        d1.path = x;
    }
    else if (noteArr[i]<currNote) {
        long l = getMoveTo(noteArr[i], 0);
        d1.dist = d1.dist + l;
        long x = 10L * d1.path + 1L;
        d1.path = x;        
    }
    Serial.println(getMoveTo(noteArr[i],1));
    currNote = noteArr[i];
    currPos = currPos+l;
    Serial.println(currNote);
  }
  while (d1.path != 0) {
    number = number * 10;
    number = number + d1.path % 10;
    d1.path = d1.path / 10;
  }
  d1.path = number;
  currPos = 0L;
  return d1;
}*/

void playNotes(int *noteArr,float *lenArr, int sizeOf) {
  Servo1.attach(handPins[0], 45);
  Servo2.attach(handPins[1], 45);
  Servo3.attach(handPins[2], 45);
  Servo1.setEasingType(EASE_CUBIC_IN_OUT); // EASE_LINEAR is default
  Servo2.setEasingType(EASE_CUBIC_IN_OUT); // EASE_LINEAR is default
  Servo3.setEasingType(EASE_CUBIC_IN_OUT); // EASE_LINEAR is default

  for(int i =0; i<sizeOf; i++) {
    Serial.print("note:");
    Serial.println(noteArr[i]);
    Serial.print("len:");
    Serial.println(lenArr[i]); 
  }
  long d = newShortPath(noteArr, sizeOf);
  
  Serial.print("dist");

  Serial.println(d);
  long g = d;
  int handIdxArr[sizeOf];
  for (int i = 0; i<sizeOf; i++) {
    handIdxArr[i]= g%10 - 1;
    g = g/10;
  }
  for (int i=0; i<sizeOf; i++) {
    Serial.print("servo");
    Serial.println(handIdxArr[i]);
    playNote(noteArr[i],lenArr[i], handIdxArr[i]); 
  }
  Serial.println("zeroing");
  for (int i=0; i<currPos+2*keyLen*stepsPerMM; i++) {
      digitalWrite(DIR,LOW);
      digitalWrite(ENA,HIGH);
      digitalWrite(PUL,HIGH);
      delayMicroseconds(50);
      digitalWrite(PUL,LOW);
      delayMicroseconds(50);
  }
  Serial.println("done zeroing");
  currPos = -2*keyLen*stepsPerMM;
}
/*
long getMoveTo(int note, int handIdx) {
  Serial.println(note);
  Serial.println(handIdx);
  long moveto = 0;
  long motor_offset = handArr[handIdx];
  if (note == 0) {
    moveto = -1;
  }
  else if (note<=35) {
    moveto = (keyLen*(note-1))*stepsPerMM + motor_offset - currPos;
    if (moveto <0){
      moveto = moveto = keyLen*stepsPerMM;
    }
  }
  return moveto;
}*/


void playNote(int note, float timeToPlay, int handIdx) {

  long moveto = getMoveTo(note, handIdx);
  Serial.print("note:");
  Serial.println(note);
  Serial.print("moveto:");
  Serial.println(moveto);
  if (moveto>0) {
    digitalWrite(DIR,HIGH);
  }
  else {
    digitalWrite(DIR,LOW);
  }

  for (int i=0; i<moveto; i++) {
      digitalWrite(ENA,HIGH);
      digitalWrite(PUL,HIGH);
      delayMicroseconds(50);
      digitalWrite(PUL,LOW);
      delayMicroseconds(50);
  }
  currPos = currPos + moveto;
  
  Serial.print("current position:");
  Serial.println( currPos);
  
  unsigned long starter = millis();
  if (handIdx ==0) {
    Servo1.easeTo(120, 200);
    delay(timeToPlay/10);
    Servo1.easeTo(80, 200);
    //myservo.write(70);
  }
  else if (handPins[handIdx] == handPins[1]) {
    Servo2.easeTo(120, 200);
    delay(timeToPlay/10);
    Servo2.easeTo(80, 200);
  }
  else if (handPins[handIdx] == handPins[2]) {
    Servo3.easeTo(120, 200);
    delay(timeToPlay/10);
    Servo3.easeTo(80, 200);
  }
  delay(1000);
  Serial.println("done");
}
