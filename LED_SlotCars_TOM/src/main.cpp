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
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// #include <SoundEngine_VS1053.hpp>

#define ARRAY_SIZE(array) ((sizeof(array)) / (sizeof(array[0])))
// #define NPIXELS 658 // MAX LEDs actives on strip
#define NPIXELS 280 // MAX LEDs actives on strip

// Pins Arduino Day 19 version
#define PIN_LED 17  // R 500 ohms to DI pin for WS2812 and WS2813, for WS2813 BI pin of first LED to GND  ,  CAP 1000 uF to VCC 5v/GND,power supplie 5V 20


Adafruit_NeoPixel track = Adafruit_NeoPixel(NPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);
SoftwareSerial mySoftwareSerial(18, 19); // RX, TX
DFRobotDFPlayerMini myDFPlayer;



#define RED track.Color(255, 0, 0)
#define GREEN track.Color(0, 255, 0)
#define BLUE track.Color(0, 0, 255)
#define YELLOW track.Color(255, 255, 0)
#define PURPLE track.Color(178, 102, 255)
#define PINK track.Color(204, 0, 102)
#define ORANGE track.Color(204, 102, 0)
#define LIGHT_BLUE track.Color(153, 255, 255)

#define NUM_PLAYERS 2

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
int PIN_INPUTS[] = {A0,A1};
uint32_t COLORS[] = {
  RED,
  BLUE,
};

int corners[] = {15, 45, 90, 120, 150, 180, 225, 270};
byte cornerLength = 2;

byte leader = 0;
byte loop_max = 5; // total laps race

#define ACEL 0.03
#define KF 0.015 // friction constant
#define KG 0.003 // gravity constant
#define CRASH_WAIT_TIME 3000
#define MAX_CORNER_SPEED 110
#define MAX_SPEED 500

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

// SoundEngine_VS1053 *soundEngine = new SoundEngine_VS1053();
//------------------------------------------------
void updateRacerLocation(Racer *racer);
void start_race();
void setTrackPixel(uint16_t n, uint32_t c);
void clearTrackPixels();
void updateTrackPixels();
void setupTrackPixels();

//------------------------------------------------
void setupTrackPixels(){
  track.begin();
}


void updateTrackPixels(){
  track.show();
}

void clearTrackPixels(){
  track.clear();
}

void setTrackPixel(uint16_t n, uint32_t c){
    track.setPixelColor(n, c);
}


//------------------------------------------------
void setup() {
  Serial.begin(9600);

  mySoftwareSerial.begin(9600);

  // if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
  //   Serial.println(F("Unable to begin:"));
  //   Serial.println(F("1.Please recheck the connection!"));
  //   Serial.println(F("2.Please insert the SD card!"));
  //   while(true);
  // }
  // Serial.println(F("DFPlayer Mini online."));
  // myDFPlayer.volume(30); 
  // myDFPlayer.next();

  // delay(2000);
  // myDFPlayer.pause();  //pause the mp3


  setupTrackPixels();
  

  // // TODO : add sound - currently board is crashing   
  // // soundEngine->begin();
  // // soundEngine->playSoundWithIndex(1);


  for (int i = 0; i < NUM_PLAYERS; i++) {
    pinMode(PIN_INPUTS[i], INPUT_PULLUP);
    racers[i] = newRacer(PIN_INPUTS[i], COLORS[i]);
  }

  start_race();

}

void start_race() { 
  
  for (int i = 0; i < NPIXELS; i++) {
    setTrackPixel(i, track.Color(0, 0, 0));
  };
  updateTrackPixels();

  if(true){
    setTrackPixel(12, track.Color(0, 255, 0));
    setTrackPixel(11, track.Color(0, 255, 0));
    updateTrackPixels();
    delay(2000);
    setTrackPixel(12, track.Color(0, 0, 0));
    setTrackPixel(11, track.Color(0, 0, 0));
    setTrackPixel(10, track.Color(255, 255, 0));
    setTrackPixel(9, track.Color(255, 255, 0));
    updateTrackPixels();
    delay(2000);
    setTrackPixel(9, track.Color(0, 0, 0));
    setTrackPixel(10, track.Color(0, 0, 0));
    setTrackPixel(8, track.Color(255, 0, 0));
    setTrackPixel(7, track.Color(255, 0, 0));
    updateTrackPixels();
    delay(2000);
  };
}

void resetRacers() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    racers[i].speed = 0;
    racers[i].location = 0;
    racers[i].lapNum = 0;
  }
}

void drawCar(Racer racer) {
  for (int i = 0; i <= racer.lapNum; i++) {
    setTrackPixel((int)racer.location , racer.color);
  };
}
void loopx() {
}
void loop() {
  clearTrackPixels();

  // light up the whole track
  for (int i = 0; i < NPIXELS; i++) {
    setTrackPixel(i, track.Color(1,1,1));
  };

  // show corners
  for (uint8_t i = 0; i < ARRAY_SIZE(corners); i++) {
    int cornerStart = corners[i] - cornerLength;
    int cornerEnd = corners[i] + cornerLength;
    int cornerSize = cornerEnd - cornerStart;
    for(int j = 0; j < cornerSize; j++){
      setTrackPixel(cornerStart + j, YELLOW);
    }
  }

  int winner = -1;
  for (int i = 0; i < NUM_PLAYERS; i++) {
    updateRacerLocation(&(racers[i]));
    if (racers[i].lapNum > loop_max) {
      winner = i;
    }
  }
  
  if (winner != -1) {
    for (int i = 0; i < NPIXELS; i++) {
      setTrackPixel(i, racers[winner].color);
    };
    updateTrackPixels();
    delay(2000);
    resetRacers();
    start_race();
  }
  
  if ((millis() & 512) == (512 * draworder)) {
    draworder = draworder == 0 ? 1 : 0;
  };

  // TODO : allow for n players
  if (draworder == 0) {
    drawCar(racers[0]);
    drawCar(racers[1]);
  } else {
    drawCar(racers[1]);
    drawCar(racers[0]);
  }

  updateTrackPixels();
  delay(tdelay);

}

void updateRacerLocation(Racer *racer) {

  if (racer->crashWait == 0) {
    if (digitalRead(racer->pin) == HIGH) {
      racer->flag_sw = 0;
      racer->speed += ACEL;
    } else if (racer->flag_sw == 0) {
      racer->flag_sw = 1;
    };

    int prevLoc = (int)racer->location;

    racer->speed -= racer->speed * KF;
    racer->location += racer->speed;
    int loc = (int)racer->location;

    for(uint8_t i=0; i < ARRAY_SIZE(corners); i++){
      if(corners[i] > (prevLoc - ( 2 * cornerLength)) && 
        corners[i] <= loc && 
        (racer->speed * 100) >= MAX_CORNER_SPEED ){
        racer->location = loc;
        racer->crashTimestamp = millis();
        racer->crashWait = CRASH_WAIT_TIME;
        racer->speed = 0;
      }
    }
  } else {
    long timeSinceCrash = millis() - racer->crashTimestamp;
    racer->crashWait = max(racer->crashWait - timeSinceCrash, 0);
  }

  if (racer->location >= NPIXELS) {
    racer->lapNum++;
    racer->location = 0;
  }
}
