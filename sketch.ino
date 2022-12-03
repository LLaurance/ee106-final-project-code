#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT();

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
};

struct Key silence = (struct Key) {"   ", -1.0, 9999.0};

#define NUM_KEYS 55

// Array of all keys
struct Key keys[NUM_KEYS] = {
  (struct Key) {"low", 65.0, 106.87},

  (struct Key) {"A 2", 106.87, 113.22},
  (struct Key) {"A#2", 113.22, 119.96},
  (struct Key) {"B 2", 119.96, 127.09},

  (struct Key) {"C 3", 127.09, 134.65},
  (struct Key) {"C#3", 134.65, 142.65},
  (struct Key) {"D 3", 142.65, 151.13},
  (struct Key) {"D#3", 151.13, 160.12},
  (struct Key) {"E 3", 160.12, 169.64},
  (struct Key) {"F 3", 169.64, 179.73},
  (struct Key) {"F#3", 179.73, 190.42},
  (struct Key) {"G 3", 190.42, 201.74},
  (struct Key) {"G#3", 201.74, 213.74},
  (struct Key) {"A 3", 213.74, 226.45},
  (struct Key) {"A#3", 226.45, 239.91},
  (struct Key) {"B 3", 239.91, 254.18},

  (struct Key) {"C 4", 254.18, 269.29},
  (struct Key) {"C#4", 269.29, 285.30},
  (struct Key) {"D 4", 285.30, 302.27},
  (struct Key) {"D#4", 302.27, 320.24},
  (struct Key) {"E 4", 320.24, 339.29},
  (struct Key) {"F 4", 339.29, 359.46},
  (struct Key) {"F#4", 359.46, 380.84},
  (struct Key) {"G 4", 380.84, 403.48},
  (struct Key) {"G#4", 403.48, 427.47},
  (struct Key) {"A 4", 427.47, 452.89},
  (struct Key) {"A#4", 452.89, 479.82},
  (struct Key) {"B 4", 479.82, 508.36},
  
  (struct Key) {"C 5", 508.36, 538.58},
  (struct Key) {"C#5", 538.58, 570.61},
  (struct Key) {"D 5", 570.61, 604.54},
  (struct Key) {"D#5", 604.54, 640.49},
  (struct Key) {"E 5", 640.49, 678.57},
  (struct Key) {"F 5", 678.57, 718.92},
  (struct Key) {"F#5", 718.92, 761.67},
  (struct Key) {"G 5", 761.67, 806.96},
  (struct Key) {"G#5", 806.96, 854.95},
  (struct Key) {"A 5", 854.95, 905.79},
  (struct Key) {"A#5", 905.79, 959.65},
  (struct Key) {"B 5", 959.65, 1016.71},
  
  (struct Key) {"C 6", 1016.71, 1077.17},
  (struct Key) {"C#6", 1077.17, 1141.22},
  (struct Key) {"D 6", 1141.22, 1209.08},
  (struct Key) {"D#6", 1209.08, 1280.97},
  (struct Key) {"E 6", 1280.97, 1357.15},
  (struct Key) {"F 6", 1357.15, 1437.85},
  (struct Key) {"F#6", 1437.85, 1523.34},
  (struct Key) {"G 6", 1523.34, 1613.93},
  (struct Key) {"G#6", 1613.93, 1709.90},
  (struct Key) {"A 6", 1709.90, 1811.57},
  (struct Key) {"A#6", 1811.57, 1919.29},
  (struct Key) {"B 6", 1919.29, 2033.42},
  (struct Key) {"C 7", 2033.42, 2154.33},

  (struct Key) {"hi ", 2154.33, 9999.0},

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

  // Wait for button release
  tft.drawString("let go", 30, 100);

  while (digitalRead(WIO_KEY_C) == LOW) {
    delay(100);
  }
  spr.pushSprite(0, 0);
  Serial.println("Button released");
  Serial.println("");

  delay(500);

  // Initialize frequency array
  int numSamples = round(PROCESSING_DURATION * SAMPLE_RATE / FRAME_SIZE) + 1;   // I don't actually know how large this should be
  double peakFrequencies[numSamples];
  struct Key peakKeys[numSamples];
  for (int i = 0; i < numSamples; i += 1) {
    peakFrequencies[i] = 0;   // just to make sure
  }

  int sampleIndex = 0;

  Serial.println("Begin processing");
  Serial.println("");

  // Get current time for countdown
  time = millis();

  // Live DSP for PROCESSING_DURATION
  while (millis() - time < PROCESSING_DURATION * 1000) {
    int totalAmplitude = 0;
    
    // Sampling
    for (int i = 0; i < FRAME_SIZE; i += 1) {
      // Fetch mic input
      int mic_input = analogRead(WIO_MIC);
      
      // Store input
      vReal[i] = mic_input;
      vImag[i] = 0;

      totalAmplitude += mic_input;

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

    Serial.println(peakFrequency);
    spr.pushSprite(0, 0);
    struct Key *peakKey = freqToKey(peakFrequency);
    tft.drawString(peakKey->name, 30, 100);
    tft.drawString(String((millis() - time) / 1000), 220, 160);
    
    // Record peak frequency
    peakFrequencies[sampleIndex] = peakFrequency;
    peakKeys[sampleIndex] = *peakKey;
    sampleIndex += 1;
  }

  delay(100);
  
  // TODO
  // Aggregate frequencies into keys
  int currFrequency = 0;
  unsigned long currTime = 0;
  for (int i = 0; ; i += 1) {
    currFrequency = peakFrequencies[i];
    if (currFrequency == 0) {
      break;
    }

    currTime = i * framePeriod * FRAME_SIZE;
  }
}

// Convert FREQ to closest KEY
struct Key *freqToKey(float freq) {
  // Special frequency for silence
  if (freq == -1) {
    return &silence;
  }

  for (int i = 0; i < NUM_KEYS; i += 1) {
    struct Key *key = &keys[i];
    if ((key->freqLow < freq) && (freq < key->freqHigh)) {
      return key;
    }
  }
}
