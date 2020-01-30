// --------------- INCLUDES ---------------

#include "FastLED.h"


// --------------- STYLE SETTINGS ---------------

// Minimum time before the mode can be changed again (in milliseconds)
#define CHANGE_MODE_TIME_MIN 5000

// Maximum time before the mode can be changed again (in milliseconds)
#define CHANGE_MODE_TIME_MAX 15000

// To how many sound levels should the volume be mapped?
#define AMOUNT_LEVELS 20

// Complete color palette that is circled through
CRGB colors[] = {
    CRGB(120, 0, 0),
    CRGB(90, 30, 0),
    CRGB(60, 60, 0),
    CRGB(30, 90, 0),
    CRGB(0, 120, 0),
    CRGB(0, 90, 30),
    CRGB(0, 60, 60),
    CRGB(0, 30, 90),
    CRGB(0, 0, 120),
    CRGB(30, 0, 90),
    CRGB(60, 0, 60),
    CRGB(90, 0, 30)
    };


// --------------- HARDWARE SETTINGS ---------------

// Amount of different modes
#define NUM_MODES 8

// Amount of different colors
#define AMOUNT_COLORS 12

// How many leds in your strip?
#define NUM_LEDS 300

// Amount of samples to calculate current volunme
#define VOLUME_SAMPLES 20

// Pin of the LED strip
#define DATA_PIN 5


// --------------- GLOBAL VARIABLES ---------------

// Led strip array
CRGB leds[NUM_LEDS];

// Current volume
int volume;

// Last x samples of the volume
int volumes[20];

// Current sound level
int level = 0;

// Current average sound level
float average = 0;

// True whenever the average is exceed for the first time
bool averageExceed = false;

// True if the sound level jumped up
bool levelJump = false;

// Time the next jump can happen again
long nextJump = 0;

// True if the color should be changed due to a hard level increase
int color = 0;

// Current mode for different light styles
int mode = 0;


// --------------- PROGRAM ---------------

void setup() {
  // Start communication to LEDs and USB
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);

  // Clear led stripe
  turnOffAllLights();
}

void loop() {
  // Get current volume
  volume = getVolume(VOLUME_SAMPLES);
  average = getAverage(volume);

  // mode = 7;

  // Convert volume to level
  if(mode == 1 || mode == 2) {
    level = getLevel(volume, true);
  } else {
    level = getLevel(volume, false);
  }
  
  // Change color and (eventually) mode on level jump
  if(levelJump) {
    // Change color
    color = nextColor();

    // Change mode eventually
    if(millis() > nextJump) {
      mode = randomMode();
      turnOffAllLights();

      // Set time for next jump
      nextJump = millis() + random(CHANGE_MODE_TIME_MIN, CHANGE_MODE_TIME_MAX);
    }
  }

  // Play one mode
  playMode(mode);
  
  // Send debug information
  debug(volume);

  FastLED.show();
}

/**
 * This mode makes a pulse
 */
void modePulse(int level, CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
      float brightness = float(level) / float(AMOUNT_LEVELS);
      leds[i] = setBrightnessOfColor(color, brightness);    
  }
}

/**
 * This mode displays a vu meter
 */
void modeVUMeter(int level, CRGB color) {
  int ledsPerLevel = (NUM_LEDS / AMOUNT_LEVELS) / 2;
  
  for (int i = 0; i < NUM_LEDS; i++) {    
    if(i < level * ledsPerLevel) {
      leds[i] = color;
    } else {
      leds[i] = CRGB(0, 0, 0);
    }
  }
}

/**
 * This mode displays a vu meter
 */
void modeVUMeterMirrored(int level, CRGB color) {
  int ledsPerLevel = (NUM_LEDS / AMOUNT_LEVELS) / 2;
  
  for (int i = 0; i < NUM_LEDS / 2; i++) {    
    if(i < level * ledsPerLevel) {
      leds[i] = color;
      leds[NUM_LEDS - 1 - i] = color;
    } else {
      leds[i] = CRGB(0, 0, 0);
      leds[NUM_LEDS - 1 - i] = CRGB(0, 0, 0);
    }
  }
}

/**
 * This mode "shoots" lights
 */
void modeShooting(int volume, CRGB color) {
  int speed = 10;
  
  for(int i = NUM_LEDS - 1; i > 0; i--) {
    leds[i] = leds[i - speed];
  }

  if(volume > average * 1.15) {
    for(int i = 0; i < speed; i++) {
      leds[i] = color;
    }
  } else {
    for(int i = 0; i < speed; i++) {
      leds[i] = CRGB(0, 0, 0);
    }
  }
}

/**
 * This mode "shoots" lights
 */
void modeShootingMirrored(int volume, CRGB color) {
  int speed = 10;

  for(int i = 0; i < NUM_LEDS - 1; i++) {
    leds[i] = leds[i + speed];
  }

  if(volume > average * 1.15) {
    for(int i = 0; i < speed; i++) {
      leds[NUM_LEDS -1 - i] = color;
    }
  } else {
    for(int i = 0; i < speed; i++) {
      leds[NUM_LEDS -1 - i] = CRGB(0, 0, 0);
    }
  }
}

/**
 * This mode flashes the leds
 */
void modeFlashing(int volume, CRGB color) {
  average = average * 1.15;

  if(volume > average) {
    for(int i = 0; i < NUM_LEDS; i++) {
      leds[i] = color;
    }
  } else {
    for(int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0, 0, 0);
    }
  }
}

/**
 * This mode makes a pulse
 */
void modeFlashPulse(int volume, CRGB color) {
  if(volume > average * 1.15 && averageExceed == false) {
    averageExceed = true;

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = color;   
    }
  } else {
    if(volume < average * 1.15) {
      averageExceed = false;
    }

    for (int i = 0; i < NUM_LEDS; i++) {
      CRGB colorTmp = leds[i];
      leds[i] = setBrightnessOfColor(leds[i], 0.7);   
    }
  }
}

/**
 * This mode makes a pulse
 */
void modeFlashPulseSmall(int volume, CRGB color) {
  if(volume > average * 1.15 && averageExceed == false) {
    averageExceed = true;

    int start = random(5, NUM_LEDS - 20);

    for (int i = start; i < start + 10; i++) {
      leds[i] = color;
    }
  } else {
    if(volume < average * 1.15) {
      averageExceed = false;
    }

    for (int i = 0; i < NUM_LEDS; i++) {
      CRGB colorTmp = leds[i];
      leds[i] = setBrightnessOfColor(leds[i], 0.7);   
    }
  }
}

/**
 * Plays the given mode
 */
void playMode(int mode) {
  if(mode == 0) {
    modePulse(level, colors[color]);
  } else if(mode == 1) {
    modeVUMeter(level, colors[color]);
  } else if(mode == 2) {
    modeVUMeterMirrored(level, colors[color]);
  } else if(mode == 3) {
    modeShooting(volume, colors[color]);
  } else if(mode == 4) {
    modeShootingMirrored(volume, colors[color]);
  }else if(mode == 5) {
    modeFlashing(volume, colors[color]);
  } else if(mode == 6) {
    modeFlashPulse(volume, colors[color]);
  } else if(mode == 7) {
    modeFlashPulseSmall(volume, colors[color]);
  }
}

/**
 * Change color of the color
 */
CRGB setBrightnessOfColor(CRGB color, float brightness) {
  CRGB colorTmp = CRGB(color.r, color.g, color.b);

  // Adjust brightness for every channel
  colorTmp.r = colorTmp.r * brightness;
  colorTmp.g = colorTmp.g * brightness;
  colorTmp.b = colorTmp.b * brightness;

  return colorTmp;
}

/**
 * Reads the current volume and returns it
 */
int getVolume(int samples) {
  int volume = 0;

  // Take the maximum volume of the last x samples
  for (int i = 0; i < samples; i++) {
    int sound = analogRead(A0);

    if (sound > volume) {
      volume = sound;
    }

    delay(1);
  }

  return volume;
}

/**
 * Maps the volume to a level
 */
int getLevel(int volume, bool smooth) {
  int levelTmp = map(volume, 550, 750, 0, AMOUNT_LEVELS);

  // Did the level jump up?
  if(levelTmp - level > 10) {
    levelJump = true;
  } else {
    levelJump = false;
  }

  // Increase or decrease level
  if (levelTmp > level) {
    if(smooth) {
      levelTmp = level + 1;
    }
  } else {
    levelTmp = level - 1;
  }

  // Avoid negative level
  if (levelTmp < 0) {
    levelTmp = 0;
  } else if(levelTmp > AMOUNT_LEVELS) {
    levelTmp = AMOUNT_LEVELS;
  }

  return levelTmp;
}

/**
 * Calculates the average of the last 20 samples and returns it
 */
float getAverage(int volume) {
  float average = 0;

  for(int i = 19; i > 0; i--) {
    average = average + volumes[i];
    volumes[i] = volumes[i - 1];
  }

  volumes[0] = volume;
  
  average = average / 19;

  return average;
}

/**
 * Turns off all lights
 */
void turnOffAllLights() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }

  FastLED.show();
}

/**
 * Changes the mode randomly
 */
int randomMode() {
  int nextMode = random(0, NUM_MODES);

  // Make sure the new mode isn't the same as the current one
  while(nextMode == mode) {
    nextMode = random(0, NUM_MODES);
  }

  return nextMode;
}

/**
 * Circles to the next color
 */
int nextColor() {
  int nextColor = color + 1;

  if(nextColor == AMOUNT_COLORS) {
    nextColor = 0;
  }

  return nextColor;
}

/**
 * Sends some information for debugging
 */
void debug(int volume) {
  // Debug volume and level
  Serial.print(volume);
  Serial.print(" | ");
  Serial.print(average);
  Serial.print(" | ");
  Serial.print(level);
  Serial.print(" | ");
  Serial.print(color);
  Serial.print(" | ");
  Serial.println(mode);
}

