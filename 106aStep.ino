// Include the AccelStepper library:
#include <AccelStepper.h>
#include "play.h"

#define DIR 6
#define PUL 7
#define ENA 5
#define motorInterfaceType 1
#define handPin 8    
int notes[5] = {1,5,10,3,1};
float lens[5] = {0.5,0.5,0.75,0.125, 1};


void setup() {
  // Set the maximum speed and acceleration:
  Serial.begin(115200);
  pinMode (PUL, OUTPUT);
  pinMode (DIR, OUTPUT);
  pinMode (ENA, OUTPUT);
}

void loop() {
  String input = "a";
  input = Serial.readStringUntil('\n');
  Serial.println(input);
  if (input.equals("z")) {
    Serial.println("running1");
    while(!input.equals("s")){
      for (int i = 0; i<10000; i++) {
          digitalWrite(DIR,HIGH);
          digitalWrite(ENA,HIGH);
          digitalWrite(PUL,HIGH);
          delayMicroseconds(50);
          digitalWrite(PUL,LOW);
          delayMicroseconds(50);
      }
        Serial.println("done");
        input = Serial.readStringUntil('\n');
        Serial.println("f");
     }
  }
  if (input.equals("i")) {
    Serial.println("running2");
    playNotes(notes, lens, 5);
    exit;
  }
}
