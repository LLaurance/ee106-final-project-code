#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT();

#include "play.h"

#include "TFT_eSPI.h"
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);

#define FRAME_SIZE 512            // > 512: may be slow
#define SAMPLE_RATE 10000         // Hz
#define PROCESSING_DURATION 10    // seconds
#define AMPLITUDE_THRESHOLD 300   // mic input must be higher than this amplitude

unsigned int framePeriod;         // Sampling period in us
unsigned long time;               // Store time

double vReal[FRAME_SIZE];
double vImag[FRAME_SIZE];

// A key with name and frequency range
struct Key {
  char name[4];
  float freqLow;
  float freqHigh;
  int index;
};

struct Key silence = (struct Key) {"   ", -1.0, 9999.0, -1};

#define NUM_KEYS 62 - 24

// Array of all white keys (62 with black keys)
struct Key allKeys[NUM_KEYS] = {
  (struct Key) {"low", 65.0, 106.87, 100},

  (struct Key) {"A 2", 106.87, 116.54,  1},
  // (struct Key) {"A#2", 113.22, 119.96, -1},
  (struct Key) {"B 2", 116.54, 127.09,  2},

  (struct Key) {"C 3", 127.09, 138.59,  3},
  // (struct Key) {"C#3", 134.65, 142.65, -1},
  (struct Key) {"D 3", 138.59, 155.56,  4},
  // (struct Key) {"D#3", 151.13, 160.12, -1},
  (struct Key) {"E 3", 155.56, 169.64,  5},
  (struct Key) {"F 3", 169.64, 185.00,  6},
  // (struct Key) {"F#3", 179.73, 190.42, -1},
  (struct Key) {"G 3", 185.00, 207.65,  7},
  // (struct Key) {"G#3", 201.74, 213.74, -1},
  (struct Key) {"A 3", 207.65, 233.08,  8},
  // (struct Key) {"A#3", 226.45, 239.91, -1},
  (struct Key) {"B 3", 233.08, 254.18,  9},

  (struct Key) {"C 4", 254.18, 277.18, 10},
  // (struct Key) {"C#4", 269.29, 285.30, -1},
  (struct Key) {"D 4", 277.18, 311.13, 11},
  // (struct Key) {"D#4", 302.27, 320.24, -1},
  (struct Key) {"E 4", 311.13, 339.29, 12},
  (struct Key) {"F 4", 339.29, 370.00, 13},
  // (struct Key) {"F#4", 359.46, 380.84, -1},
  (struct Key) {"G 4", 370.00, 415.30, 14},
  // (struct Key) {"G#4", 403.48, 427.47, -1},
  (struct Key) {"A 4", 415.30, 466.16, 15},
  // (struct Key) {"A#4", 452.89, 479.82, -1},
  (struct Key) {"B 4", 466.16, 508.36, 16},
  
  (struct Key) {"C 5", 508.36, 554.36, 17},
  // (struct Key) {"C#5", 538.58, 570.61, -1},
  (struct Key) {"D 5", 554.36, 622.26, 18},
  // (struct Key) {"D#5", 604.54, 640.49, -1},
  (struct Key) {"E 5", 622.26, 678.57, 19},
  (struct Key) {"F 5", 678.57, 739.99, 20},
  // (struct Key) {"F#5", 718.92, 761.67, -1},
  (struct Key) {"G 5", 739.99, 830.61, 21},
  // (struct Key) {"G#5", 806.96, 854.95, -1},
  (struct Key) {"A 5", 830.61, 932.33, 22},
  // (struct Key) {"A#5", 905.79, 959.65, -1},
  (struct Key) {"B 5", 932.33, 1016.71, 23},
  
  (struct Key) {"C 6", 1016.71, 1108.73, 24},
  // (struct Key) {"C#6", 1077.17, 1141.22, -1},
  (struct Key) {"D 6", 1108.73, 1244.51, 25},
  // (struct Key) {"D#6", 1209.08, 1280.97, -1},
  (struct Key) {"E 6", 1244.51, 1357.15, 26},
  (struct Key) {"F 6", 1357.15, 1479.98, 27},
  // (struct Key) {"F#6", 1437.85, 1523.34, -1},
  (struct Key) {"G 6", 1479.98, 1661.22, 28},
  // (struct Key) {"G#6", 1613.93, 1709.90, -1},
  (struct Key) {"A 6", 1661.22, 1864.65, 29},
  // (struct Key) {"A#6", 1811.57, 1919.29, -1},
  (struct Key) {"B 6", 1864.65, 2033.42, 30},

  (struct Key) {"C 7", 2033.42, 2217.46, 31},
  // (struct Key) {"C#7", 2154.33, 2282.44, -1},
  (struct Key) {"D 7", 2217.46, 2489.69, 32},
  // (struct Key) {"D#7", 2419.16, 2561.95, -1},
  (struct Key) {"E 7", 2489.69, 2714.29, 33},
  (struct Key) {"F 7", 2714.29, 2959.96, 34},
  // (struct Key) {"F#7", 2875.69, 3046.69, -1},
  (struct Key) {"G 7", 2959.96, 3227.85, 35},

  (struct Key) {"hi ", 3227.85, 9999.0, 101},

  silence
};

void setup() {
  // Collect input from mic
  pinMode(WIO_MIC, INPUT);

  // Top-left button for starting 10s
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  
  // Calculate frame period in microseconds
  framePeriod = round(1000000 / SAMPLE_RATE);

  // Serial monitor output
  Serial.begin(115200);

  // Monitor output
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  spr.createSprite(TFT_HEIGHT, TFT_WIDTH);
  spr.fillSprite(TFT_BLACK);
  tft.setTextSize(100);
  tft.setTextColor(TFT_RED);
}

void loop() {
  // Wait until button press
  spr.pushSprite(0, 0);
  tft.drawString("idle", 80, 100);

  while (digitalRead(WIO_KEY_C) != LOW) {
    delay(100);
  }
  spr.pushSprite(0, 0);
  Serial.println("Button pressed");
  Serial.println("");

  // Initialize result arrays
  int numSamples = round(PROCESSING_DURATION * SAMPLE_RATE / FRAME_SIZE) + 1;   // I don't actually know how large this should be
  double peakFrequencies[numSamples];
  struct Key peakKeys[numSamples];
  int keyIndexSequence[numSamples];
  double durationSequence[numSamples];

  for (int i = 0; i < numSamples; i += 1) {
    peakFrequencies[i] = 0;
    peakKeys[i] = silence;
    keyIndexSequence[i] = 0;
    durationSequence[i] = FRAME_SIZE;
  }

  // Wait for button release
  tft.drawString("let go", 30, 100);

  while (digitalRead(WIO_KEY_C) == LOW) {
    delay(100);
  }
  spr.pushSprite(0, 0);
  Serial.println("Button released");
  Serial.println("");

  delay(500);

  int sampleIndex = 0;
  int sequenceIndex = -1;

  Serial.println("Begin processing");
  Serial.println("");

  // Get current time for countdown
  time = millis();

  // Live DSP for PROCESSING_DURATION
  while (millis() - time < PROCESSING_DURATION * 1000) {
    // int totalAmplitude = 0;
    
    // Sampling
    for (int i = 0; i < FRAME_SIZE; i += 1) {
      // Fetch mic input
      int mic_input = analogRead(WIO_MIC);

      // Store input
      vReal[i] = mic_input;
      vImag[i] = 0;

      // totalAmplitude += mic_input;

      // Delay
      delayMicroseconds(framePeriod);
    }

    //Serial.println(String(totalAmplitude / FRAME_SIZE));

    double peakFrequency;

    // If average input too soft, determine as silence; else find peak frequency by FFT
    /*
    if (totalAmplitude / FRAME_SIZE < AMPLITUDE_THRESHOLD) {
      peakFrequency = -1;
    } else {
      FFT.Windowing(vReal, FRAME_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.Compute(vReal, vImag, FRAME_SIZE, FFT_FORWARD);
      FFT.ComplexToMagnitude(vReal, vImag, FRAME_SIZE);
      peakFrequency = FFT.MajorPeak(vReal, FRAME_SIZE, SAMPLE_RATE);

      // lol wtf
      peakFrequency *= 0.8;
    }
    */

    FFT.Windowing(vReal, FRAME_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, FRAME_SIZE, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, FRAME_SIZE);
    peakFrequency = FFT.MajorPeak(vReal, FRAME_SIZE, SAMPLE_RATE);

    // lol wtf
    peakFrequency *= 0.8;
    // Serial.println(peakFrequency);

    spr.pushSprite(0, 0);
    struct Key *peakKey = freqToKey(peakFrequency);
    tft.drawString(peakKey->name, 30, 100);
    tft.drawString(String((millis() - time) / 1000), 220, 160);
    
    // Record peak frequency
    peakFrequencies[sampleIndex] = peakFrequency;
    peakKeys[sampleIndex] = *peakKey;

    Serial.println(peakKey->index);
    // Record to key-duration timetable
    if (keyIndexSequence[sequenceIndex] == peakKey->index) {
      durationSequence[sequenceIndex] += FRAME_SIZE;
    } else {
      sequenceIndex += 1;
      keyIndexSequence[sequenceIndex] = peakKey->index;
    }

    sampleIndex += 1;
  }

  delay(100);

  for (int i = 0; i < sequenceIndex; i += 1) {
    Serial.print(String(keyIndexSequence[i]));
    Serial.print(" ");
    Serial.println(String(durationSequence[i]));
  }

  playNotes(keyIndexSequence, durationSequence, sequenceIndex);
}

// Convert FREQ to closest KEY
struct Key *freqToKey(float freq) {
  // Special frequency for silence
  if (freq == -1) {
    return &silence;
  }

  for (int i = 0; i < NUM_KEYS; i += 1) {
    struct Key *key = &allKeys[i];
    if ((key->freqLow < freq) && (freq < key->freqHigh)) {
      return key;
    }
  }
}
