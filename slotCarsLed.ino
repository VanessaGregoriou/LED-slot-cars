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

#define MAXLED 300 // MAX LEDs actives on strip

// Pins Arduino Day 19 version
#define PIN_LED A0  // R 500 ohms to DI pin for WS2812 and WS2813, for WS2813 BI pin of first LED to GND  ,  CAP 1000 uF to VCC 5v/GND,power supplie 5V 2A

#define NUM_PLAYERS 2

/// RED color
#define RED track.Color(255, 0, 0)
/// GREEN color
#define GREEN track.Color(0, 255, 0)
/// Racer 3 color. BLUE
#define BLUE track.Color(0, 0, 255)
/// Racer 4 color. YELLOW
#define YELLOW track.Color(255, 255, 0)
/// Racer 5 color. PURPLE
#define PURPLE track.Color(178, 102, 255)
/// Racer 6 color. PINK
#define PINK track.Color(204, 0, 102)
/// Racer 7 color. ORANGE
#define ORANGE track.Color(204, 102, 0)
/// Racer 8 color. LIGHT BLUE
#define LIGHT_BLUE track.Color(153, 255, 255)

typedef struct {
  int pin;
  uint32_t color;
  byte flag_sw;
  float speed;
  float location;
  byte lapNum;
} Racer;

Racer racers[NUM_PLAYERS];

#define PIN_P1 7    // switch player 1 to PIN and GND
#define PIN_P2 6    // switch player 2 to PIN and GND
#define PIN_AUDIO 3 // through CAP 2uf to speaker 8 ohms

int NPIXELS = MAXLED; // leds on track

int win_music[] = {
    2637, 2637, 0, 2637,
    0, 2093, 2637, 0,
    3136};

byte gravity_map[MAXLED];

int TBEEP = 3;

byte leader = 0;
byte loop_max = 5; // total laps race

float ACEL = 0.2;
float kf = 0.015; // friction constant
float kg = 0.003; // gravity constant

byte draworder = 0;

unsigned long timestamp = 0;

Adafruit_NeoPixel track = Adafruit_NeoPixel(MAXLED, PIN_LED, NEO_GRB + NEO_KHZ800);

int tdelay = 5;

void set_ramp(byte H, byte a, byte b, byte c) {
  for (int i = 0; i < (b - a); i++) {
    gravity_map[a + i] = 127 - i * ((float)H / (b - a));
  };
  gravity_map[b] = 127;
  for (int i = 0; i < (c - b); i++) {
    gravity_map[b + i + 1] = 127 + H - i * ((float)H / (c - b));
  };
}

void set_loop(byte H, byte a, byte b, byte c) {
  for (int i = 0; i < (b - a); i++) {
    gravity_map[a + i] = 127 - i * ((float)H / (b - a));
  };
  gravity_map[b] = 255;
  for (int i = 0; i < (c - b); i++) {
    gravity_map[b + i + 1] = 127 + H - i * ((float)H / (c - b));
  };
}

Racer newRacer(int pin, uint32_t color) {
  return Racer{pin, color, 0, 0, 0, 0};
}

void setup() {
  for (int i = 0; i < NPIXELS; i++) {
    gravity_map[i] = 127;
  };
  track.begin();
  pinMode(PIN_P1, INPUT_PULLUP);
  pinMode(PIN_P2, INPUT_PULLUP);

  racers[0] = newRacer(PIN_P1, RED);
  racers[1] = newRacer(PIN_P2, YELLOW);

  start_race();
}

void start_race() {
  for (int i = 0; i < NPIXELS; i++) {
    track.setPixelColor(i, track.Color(0, 0, 0));
  };
  track.show();
  delay(2000);
  track.setPixelColor(12, track.Color(0, 255, 0));
  track.setPixelColor(11, track.Color(0, 255, 0));
  track.show();
  tone(PIN_AUDIO, 400);
  delay(2000);
  noTone(PIN_AUDIO);
  track.setPixelColor(12, track.Color(0, 0, 0));
  track.setPixelColor(11, track.Color(0, 0, 0));
  track.setPixelColor(10, track.Color(255, 255, 0));
  track.setPixelColor(9, track.Color(255, 255, 0));
  track.show();
  tone(PIN_AUDIO, 600);
  delay(2000);
  noTone(PIN_AUDIO);
  track.setPixelColor(9, track.Color(0, 0, 0));
  track.setPixelColor(10, track.Color(0, 0, 0));
  track.setPixelColor(8, track.Color(255, 0, 0));
  track.setPixelColor(7, track.Color(255, 0, 0));
  track.show();
  tone(PIN_AUDIO, 1200);
  delay(2000);
  noTone(PIN_AUDIO);
  timestamp = 0;
};

void winner_fx() {
  int msize = sizeof(win_music) / sizeof(int);
  for (int note = 0; note < msize; note++)
  {
    tone(PIN_AUDIO, win_music[note], 200);
    delay(230);
    noTone(PIN_AUDIO);
  }
};

void draw_car(Racer racer) {
  for (int i = 0; i <= racer.lapNum; i++) {
    track.setPixelColor(((word)racer.location % NPIXELS) + i, racer.color);
  };
}


void loop() {
  track.clear();
  for (int i = 0; i < NPIXELS; i++) {
    track.setPixelColor(i, track.Color(0, 0, (127 - gravity_map[i]) / 8));
  };

  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (racers[i].flag_sw == 1) && (digitalRead(racers[i].pin) == 0) {
      racers[i].flag_sw = 0;
      racers[i].speed += ACEL;
    };
    if (racers[i].flag_sw == 0) && (digitalRead(racers[i].pin) == 1) {
      racers[i].flag_sw = 1;
    };

    racers[i].speed -= racers[i].speed * kf;
    racers[i].location += racers[i].speed;

    if (racers[i].location > NPIXELS * racers[i].lapNum) {
      racers[i].lapNum++;
      tone(PIN_AUDIO, 600);
      TBEEP = 2;
    }
  }

  // if (dist1 > dist2) {
  //   leader = 1;
  // }
  // if (dist2 > dist1) {
  //   leader = 2;
  // };

  // if (loop1 > loop_max) {
  //   for (int i = 0; i < NPIXELS; i++) {
  //     track.setPixelColor(i, track.Color(0, 255, 0));
  //   };
  //   track.show();
  //   winner_fx();
  //   loop1 = 0;
  //   loop2 = 0;
  //   dist1 = 0;
  //   dist2 = 0;
  //   speed1 = 0;
  //   speed2 = 0;
  //   timestamp = 0;
  //   start_race();
  // }

  // if (loop2 > loop_max) {
  //   for (int i = 0; i < NPIXELS; i++) {
  //     track.setPixelColor(i, track.Color(255, 0, 0));
  //   };
  //   track.show();
  //   winner_fx();
  //   loop1 = 0;
  //   loop2 = 0;
  //   dist1 = 0;
  //   dist2 = 0;
  //   speed1 = 0;
  //   speed2 = 0;
  //   timestamp = 0;
  //   start_race();
  // }

  if ((millis() & 512) == (512 * draworder)) {
    draworder = draworder == 0 ? 1 : 0;
  };

  if (draworder == 0) {
    draw_car(racers[0]);
    draw_car(racers[1]);
  } else {
    draw_car(racers[1]);
    draw_car(racers[0]);
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
