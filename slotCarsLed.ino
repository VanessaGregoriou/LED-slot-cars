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
 * copies or substantial portions of the Software.

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

#define NPIXELS 300 // MAX LEDs actives on strip

// Pins Arduino Day 19 version
#define PIN_LED A0  // R 500 ohms to DI pin for WS2812 and WS2813, for WS2813 BI pin of first LED to GND  ,  CAP 1000 uF to VCC 5v/GND,power supplie 5V 2A

#define NUM_PLAYERS 2
#define MAX_SPEED 0.9

Adafruit_NeoPixel track = Adafruit_NeoPixel(NPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

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
  int location;
  byte lapNum;
  unsigned long crashTimestamp; // The timestamp when the racer last crashed.
  unsigned long crashWait; // How much time is left before the racer can start again.
} Racer;

Racer racers[NUM_PLAYERS];

// switch players to PIN and GND
int PIN_INPUTS[] = {7, 6};
uint32_t COLORS[] = {
  ORANGE,
  BLUE,
  YELLOW,
  PURPLE
};

#define PIN_AUDIO 3 // through CAP 2uf to speaker 8 ohms

int win_music[] = {
    2637, 2637, 0, 2637,
    0, 2093, 2637, 0,
    3136};

int raceMap[NPIXELS];

int TBEEP = 3;

byte leader = 0;
byte loop_max = 5; // total laps race

#define ACEL 0.03
#define KF 0.015 // friction constant
#define KG 0.003 // gravity constant
#define CRASH_WAIT_TIME 3000

byte draworder = 0;

int tdelay = 5;

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
  for (int i = 0; i < NUM_PLAYERS; i++) {
    pinMode(PIN_INPUTS[i], INPUT_PULLUP);
    racers[i] = newRacer(PIN_INPUTS[i], COLORS[i]);
  }

  start_race();
}

void start_race() {
  for (int i = 0; i < NPIXELS; i++) {
    raceMap[i] = 100;
    track.setPixelColor(i, track.Color(0, 0, 0));
  };
  raceMap[15] = MAX_SPEED;
  track.show();

    // delay(2000);
    // track.setPixelColor(12, track.Color(0, 255, 0));
    // track.setPixelColor(11, track.Color(0, 255, 0));
    // track.show();
    // tone(PIN_AUDIO, 400);
    // delay(2000);
    // noTone(PIN_AUDIO);
    // track.setPixelColor(12, track.Color(0, 0, 0));
    // track.setPixelColor(11, track.Color(0, 0, 0));
    // track.setPixelColor(10, track.Color(255, 255, 0));
    // track.setPixelColor(9, track.Color(255, 255, 0));
    // track.show();
    // tone(PIN_AUDIO, 600);
    // delay(2000);
    // noTone(PIN_AUDIO);
    // track.setPixelColor(9, track.Color(0, 0, 0));
    // track.setPixelColor(10, track.Color(0, 0, 0));
    // track.setPixelColor(8, track.Color(255, 0, 0));
    // track.setPixelColor(7, track.Color(255, 0, 0));
    // track.show();
    // tone(PIN_AUDIO, 1200);
    // delay(2000);
    // noTone(PIN_AUDIO);
  };

void resetRacers() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    racers[i].speed = 0;
    racers[i].location = 0;
    racers[i].lapNum = 0;
  }
}

void winnerFx() {
  int msize = sizeof(win_music) / sizeof(int);
  for (int note = 0; note < msize; note++) {
    tone(PIN_AUDIO, win_music[note], 200);
    delay(230);
    noTone(PIN_AUDIO);
  }
};

void drawCar(Racer racer) {
  for (int i = 0; i <= racer.lapNum; i++) {
    track.setPixelColor(((word)racer.location % NPIXELS) + i, racer.color);
  };
}

void loop() {
  track.clear();
  int winner = -1;
  for (int i = 0; i < NUM_PLAYERS; i++) {
    racers[i] = updateRacerLocation(racers[i]);
    if (racers[i].lapNum > loop_max) {
      winner = i;
    }
  }
  
  if (winner != -1) {
    for (int i = 0; i < NPIXELS; i++) {
      track.setPixelColor(i, racers[winner].color);
    };
    track.show();
    winnerFx();
    resetRacers();
    start_race();
  }

  if ((millis() & 512) == (512 * draworder)) {
    draworder = draworder == 0 ? 1 : 0;
  };

  if (draworder == 0) {
    drawCar(racers[0]);
    // drawCar(racers[1]);
  } else {
    // drawCar(racers[1]);
    drawCar(racers[0]);
  }

  track.show();
  delay(tdelay);

  if (TBEEP > 0) {
    TBEEP -= 1;
    if (TBEEP == 0) {
      noTone(PIN_AUDIO);
    }; // lib conflict !!!! interruption off by neopixel
  };
}

Racer updateRacerLocation(Racer racer) {
  if (racer.crashWait == 0) {
    if (digitalRead(racer.pin) == 0) {
      racer.flag_sw = 0;
      racer.speed += ACEL;
    } else if (racer.flag_sw == 0) {
      racer.flag_sw = 1;
    };
    racer.speed -= racer.speed * KF;

    for (int distFromLocation = 0; distFromLocation < racer.speed; distFromLocation++) {
      int location = (racer.location + distFromLocation) % NPIXELS;
      // if (location >= NPIXELS) {
      //   location -= NPIXELS;
      // }

      if (racer.speed >= raceMap[location]) {
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
    tone(PIN_AUDIO, 600);
    TBEEP = 2;
  }
  return racer;
}
