#include "Arduino.h"

typedef struct dist_path {                                                                       //struct to return both path and compare distances
  long dist;
  long path;
} dist_path;

dist_path shortestPath(int *noteArr, int startIdx, int endIdx, long motorPos);
void playNotes(int *noteArr,float *lenArr, int sizeOf);

long getMoveTo(int note, int handIdx);

void playNote(int note, float timeToPlay, int handIdx);
