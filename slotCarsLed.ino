/**
 * @file slotCarLed
 * @author Vanessa Gregoriou (vanessa.e.gregoriou@gmail.com)
 * @brief Accessible slot car racing with Arduino and WS2812b led strip. Made for TOM: Geelong 2023
 * @version 0.0.0
 * @date 25/3/2023
 * 
 * @copyright MIT License
 * Copyright (c) [year] [fullname]
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
export type first = {second} * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <Adafruit_NeoPixel.h>
#include <math.h>

#define ARRAY_SIZE(array) ((sizeof(array)) / (sizeof(array[0])))
#define NPIXELS 300 // MAX LEDs actives on strip
#define PIN_LED A0
#define PIN_START_BUTTON 11
#define NUM_PLAYERS 2
#define NUM_LAPS 2 // total laps race

Adafruit_NeoPixel track = Adafruit_NeoPixel(NPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

#define CLEAR track.Color(0, 0, 0)
#define WHITE track.Color(255, 255, 255);
#define RED track.Color(255, 0, 0)
#define GREEN track.Color(0, 255, 0)
#define BLUE track.Color(0, 0, 255)
#define YELLOW track.Color(255, 255, 0)
#define PURPLE track.Color(178, 102, 255)
#define PINK track.Color(204, 0, 102)
#define ORANGE track.Color(204, 102, 0)
#define LIGHT_BLUE track.Color(153, 255, 255)

typedef struct {
  int pin;
  uint32_t color;
  byte flag_sw;
  float speed;
  float location;
  byte lapNum;
  unsigned long crashTimestamp; // The timestamp when the racer last crashed.
  long crashWait; // How much time is left before the racer can start again.
} Racer;

Racer racers[NUM_PLAYERS];

// switch players to PIN and GND
int PIN_INPUTS[] = {7, 6};
uint32_t COLORS[] = {
  BLUE,
  RED,
};

#define PIN_AUDIO 3 // through CAP 2uf to speaker 8 ohms

int raceMap[NPIXELS];
int corners[] = {30, 70, 110, 145, 176, 210, 242, 265};

bool raceStarted = false;

#define ACEL 0.018
#define KF 0.015 // friction constant
#define KG 0.003 // gravity constant
#define CRASH_WAIT_TIME 10000
#define MAX_CORNER_SPEED 150
#define MAX_SPEED 500
#define TDELAY 5

byte draworder = 0;

Racer newRacer(int pin, uint32_t color) {
  return Racer{
      pin, 
      color, 
      /* flag_sw= */ 0,
      /* speed= */ 0,
      /* location= */ 0,
      /* lapNum= */ 0,
      /* crashTimestamp= */ 0,
      /* crashWait= */ 0};
}

void setup() {
  Serial.begin(9600);
  track.begin();
  pinMode(PIN_START_BUTTON, INPUT_PULLUP);
  for (int i = 0; i < NUM_PLAYERS; i++) {
    pinMode(PIN_INPUTS[i], INPUT_PULLUP);
    racers[i] = newRacer(PIN_INPUTS[i], COLORS[i]);
  }

  prepRace();
}

void prepRace() {
  for (int i = 0; i < NPIXELS; i++) {
    raceMap[i] = MAX_SPEED;
    track.setPixelColor(i, track.Color(0, 0, 0));
  };
  for (int i = 0; i < ARRAY_SIZE(corners); i++) {
    int cornerStart = corners[i];
    for (int j = -5; j < 5; j++) {
      raceMap[cornerStart + j] = MAX_CORNER_SPEED - (5 - abs(j)) * 10;
    }
  }
}

void resetRace() {
  raceStarted = false;
  for (int i = 0; i < NUM_PLAYERS; i++) {
    racers[i].speed = 0;
    racers[i].location = 0;
    racers[i].lapNum = 0;
  }
}

void winnerFx() {
  // int msize = sizeof(win_music) / sizeof(int);
  // for (int note = 0; note < msize; note++) {
  //   tone(PIN_AUDIO, win_music[note], 200);
  //   delay(230);
  //   noTone(PIN_AUDIO);
  // }
};

void drawCar(Racer racer) {
  for (int i = 0; i <= racer.lapNum; i++) {
    uint32_t color = racer.color;
    if (racer.crashWait > 0 && racer.crashWait % 3000 < 1500) {
      color = WHITE;
    }
    track.setPixelColor(((word)racer.location % NPIXELS) + i, color);
  }
}

void loop() {
  track.clear();
  if (!raceStarted) {
    bool blinkOn = true;
    while (!buttonPressed(PIN_START_BUTTON)) {
      if (blinkOn) {
        showCorners();
      } else {
        track.clear();
        track.show();
      }
      // blinkOn = !blinkOn;
      delay(500);
    }
    raceStarted = true;
    beginCountdown();
  } else {
    drawRace();
  }
  delay(TDELAY);
}

void showCorners() {
  for (int i = 0; i < ARRAY_SIZE(corners); i++) {
    int cornerStart = corners[i];
    for (int j = -5; j < 5; j++) {
      track.setPixelColor(cornerStart + j, PINK);
    }
  }
  track.show();
}

void beginCountdown() {
  track.clear();
  // TODO: start the sound effects
  track.setPixelColor(9, RED);
  track.setPixelColor(8, RED);
  track.setPixelColor(7, RED);
  track.setPixelColor(6, ORANGE);
  track.setPixelColor(5, ORANGE);
  track.setPixelColor(4, ORANGE);
  track.setPixelColor(3, GREEN);
  track.setPixelColor(2, GREEN);
  track.setPixelColor(1, GREEN);
  track.show();
  delay(1500);

  track.setPixelColor(9, CLEAR);
  track.setPixelColor(8, CLEAR);
  track.setPixelColor(7, CLEAR);
  track.show();
  delay(1500);

  track.setPixelColor(6, CLEAR);
  track.setPixelColor(5, CLEAR);
  track.setPixelColor(4, CLEAR);
  track.show();
  delay(1500);

  track.setPixelColor(3, CLEAR);
  track.setPixelColor(2, CLEAR);
  track.setPixelColor(1, CLEAR);
  track.show();
}

void drawRace() {
  int winner = -1;
  for (int i = 0; i < NUM_PLAYERS; i++) {
    racers[i] = updateRacerLocation(racers[i]);
    if (racers[i].lapNum >= NUM_LAPS) {
      winner = i;
    }
  }
  
  if (winner != -1) {
    for (int i = 0; i < NPIXELS; i++) {
      track.setPixelColor(i, racers[winner].color);
    };
    track.show();
    winnerFx();
    while (!buttonPressed(PIN_START_BUTTON));
    resetRace();
  }

  if ((millis() & 512) == (512 * draworder)) {
    draworder = draworder == 0 ? 1 : 0;
  };

  if (draworder == 0) {
    drawCar(racers[0]);
    drawCar(racers[1]);
  } else {
    drawCar(racers[1]);
    drawCar(racers[0]);
  }

  track.show();
}

Racer updateRacerLocation(Racer racer) {
  if (racer.crashWait == 0) {
    if (buttonPressed(racer.pin)) {
      racer.flag_sw = 0;
      racer.speed += ACEL;
    } else if (racer.flag_sw == 0) {
      racer.flag_sw = 1;
    };
    racer.speed -= racer.speed * KF;

    for (int distFromLocation = 0; distFromLocation < racer.speed; distFromLocation++) {
      int location = round(racer.location + distFromLocation) % NPIXELS;

      if (racer.speed * 100 >= raceMap[location]) {
        racer.location = location;
        racer.crashTimestamp = millis();
        racer.crashWait = CRASH_WAIT_TIME;
        racer.speed = 0;
        break;
      }
    }
    if (racer.crashWait == 0) {
      racer.location += racer.speed;
    }
  } else {
    long timeSinceCrash = millis() - racer.crashTimestamp;
    racer.crashWait = max(racer.crashWait - timeSinceCrash, 0);
  }

  if (racer.location >= NPIXELS) {
    racer.lapNum++;
    racer.location = 0;
  }
  return racer;
}

bool buttonPressed(int pin) {
  return digitalRead(pin) == 0;
}