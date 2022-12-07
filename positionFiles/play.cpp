#include "play.h"
#include <Servo.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define DIR 13
#define PUL 15
#define ENA 37
#define motorInterfaceType 1
long stepsPerMM = 400L; 
int keyLen = 20;
int handArr[3] = {0,-keyLen*stepsPerMM,-2*keyLen*stepsPerMM}; //index 0=left servo's offset
int handPins[3] = {9,10,11}; //index 0= left servo's pin, etc
int numWhite = 27;                  
long currPos = 0L;

// Create a new instance of the AccelStepper class:

Servo myservo;
Servo myservo1;
Servo myservo2;

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

void playNotes(int *noteArr,float *lenArr, int sizeOf) {
  myservo.write(60);
  myservo1.write(60);
  myservo2.write(90);
  dist_path d = shortestPath(noteArr, 0, sizeOf, 0);
  Serial.print("dist");
  Serial.println(d.dist);
  long g = d.path;
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
  
}

long getMoveTo(int note, int handIdx) {
  long moveto = 0;
  long motor_offset = handArr[handIdx];
  if (note<=35) {
    moveto = (keyLen*note - (keyLen/2))*stepsPerMM + motor_offset - currPos;
  }
  else {
    note = note-33;
    int groupPos = note%5;
    int group = note/5;
    int startGrp = keyLen+(group*7*keyLen);
    if (groupPos<=1){
      moveto = (startGrp+keyLen*groupPos)*stepsPerMM + motor_offset;
    }
    else {
      moveto = (keyLen*3 + startGrp+keyLen*(groupPos-2))*stepsPerMM + motor_offset;
    }
  }
  return moveto;
}


void playNote(int note, float timeToPlay, int handIdx) {

  long moveto = getMoveTo(note, handIdx);
  Serial.print("note:");
  Serial.println(note);
  Serial.print("moveto:");
  Serial.println(moveto);
  if (moveto>0) {
    for (int i=0; i<moveto; i++) {
      digitalWrite(DIR,LOW);
      digitalWrite(ENA,HIGH);
      digitalWrite(PUL,HIGH);
      delayMicroseconds(50);
      digitalWrite(PUL,LOW);
      delayMicroseconds(50);
    }
  }
  else {
    for (int i=0; i<-moveto; i++) {
        digitalWrite(DIR,HIGH);
        digitalWrite(ENA,HIGH);
        digitalWrite(PUL,HIGH);
        delayMicroseconds(50);
        digitalWrite(PUL,LOW);
        delayMicroseconds(50);
    }
  }

  currPos = currPos + moveto;
  Serial.print("current position:");
  Serial.println( currPos);
  if (handPins[handIdx] == handPins[0]) {
    myservo.write(60);
  }
  else if (handPins[handIdx] == handPins[1]) {
    myservo1.write(60);
  }
   else if (handPins[handIdx] == handPins[2]) {
    myservo2.write(90);
  }
  unsigned long starter = millis();
  while ((millis() - starter) < timeToPlay*1000) {
    continue;
  }
  Serial.println(0);
  if (handPins[handIdx] == handPins[0]) {
    myservo.write(100);
  }
  else if (handPins[handIdx] == handPins[1]) {
    myservo1.write(120);
  }
  else if (handPins[handIdx] == handPins[2]) {
    myservo2.write(0);
  }
  Serial.println("done");
}
