



/**
 * 
 *  This is designed to operate the Ride Start box art project. Uses a multitude
 *  of switches, rotary encoders, & buttons to detect settings and plays audio 
 *  bites and flashes the ride start LED accodringly. This is a replacement for 
 *  a slower implementation that was originally written in Python on Raspberry Pi.
 *  This art project is designed to simulate the feeling of operating a theme park
 *  attraction control panel!
 *    -Manny Batt
 * 
**/ 




// ***************************************
// ********** Global Variables ***********
// ***************************************


//GPIO //https://www.makerguides.com/wp-content/uploads/2020/10/arduino-nano-pinout.png
#define indicators 19
#define rotaryClock 18
#define rotaryData 4
#define rotaryButton 5
#define chooserUp 6
#define chooserDown 7
#define switchL 8
#define switchR 9
#define button 10
#define light 11
#define rx 3
#define tx 2
#define doorSensor 12

//RGB
#include "FastLED.h"
#define NUM_LEDS 6
struct CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100

//MP3 Player
#include <DFPlayer_Mini_Mp3.h>
#include <SoftwareSerial.h>
SoftwareSerial mp3Serial(rx, tx);  //Pins for MP3 Player Serial (RX, TX)

//Variables
int spiel = -1;
int lastSpiel = -1;
int volume = 26;
int volChangeOccured = 0;
int previousVol = 0;
int previousPreviousVol = 0;
int doorState = 0;
int lastDoorState = -1;
int partyMode = 0;
int greenMode = 0;
int lightState = 1;
int startPeriod;

int chanceTime;
int failChance = 0;
int memeCounter = 1;

int currentStateCLK;
int previousStateCLK;

int k = 0;
int kDir = 1;




// ***************************************
// *************** Setup *****************
// ***************************************


void setup() {

  //Initialize Input & Output
  pinMode(rotaryClock, INPUT);
  pinMode(rotaryData, INPUT);
  pinMode(rotaryButton, INPUT);
  pinMode(chooserUp, INPUT_PULLUP);
  pinMode(chooserDown, INPUT_PULLUP);
  pinMode(switchL, INPUT_PULLUP);
  pinMode(switchR, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  pinMode(light, OUTPUT);
  pinMode(doorSensor, INPUT_PULLUP);
  digitalWrite(light, LOW);


  //Initialize RGB
  FastLED.addLeds<WS2811, indicators, RGB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
  FastLED.clear();
  leds[0].setRGB(0, 60, 0);
  leds[1].setRGB(0, 60, 0);
  leds[2].setRGB(0, 0, 0);
  leds[3].setRGB(0, 0, 0);
  leds[4].setRGB(0, 0, 0);
  leds[5].setRGB(0, 0, 0);
  FastLED.show();
  delay(200);
  leds[0].setRGB(0, 0, 0);
  leds[1].setRGB(0, 0, 0);
  FastLED.show();
  delay(200);
  leds[0].setRGB(0, 60, 0);
  leds[1].setRGB(0, 60, 0);
  FastLED.show();
  delay(200);
  leds[0].setRGB(0, 0, 0);
  leds[1].setRGB(0, 0, 0);
  FastLED.show();

  //Start random seed and Serial
  Serial.begin(9600);
  Serial.println("** Starting the ride **");
  randomSeed(analogRead(0));

  //MP3
  mp3Serial.begin (9600);
  mp3_set_serial (mp3Serial);
  delay(1000);
  mp3_set_volume (volume);

  //Rotary
  previousStateCLK = digitalRead(rotaryClock);
  mp3_play_physical(24); // Lumos
  startPeriod = millis();

  //Fade in button light
  spielSelector();
  indicatorPaint();
  FastLED.setBrightness(0);
  FastLED.show();
  int j = 0;
  //Serial.println(sTime);
  Serial.println(millis());
  int sTime = millis();
  int t = millis();
  for (; t - sTime < 1000; ) {
    j += 1;
    if (j >= 255) {
      break;
    }
    Serial.println(j);
    analogWrite(light, j);
    FastLED.setBrightness(j);
    FastLED.show();
    t = millis();
  }

  //Check for held press for "Green Mode" button press
  int q = digitalRead(rotaryButton);
  delay(10);
  while (digitalRead(rotaryButton) == 0) {
    //wait for release
  }
  
  if (q == 0) {
    Serial.println("RotaryBtn Pressed!, Initializing Green Mode");
    greenMode = 1;
    FastLED.clear();
    FastLED.show();
    leds[0].setRGB(0, 255, 0);
    leds[1].setRGB(0, 255, 0);
    leds[2].setRGB(255, 0, 0);
    leds[3].setRGB(255, 0, 0);
    leds[4].setRGB(255, 0, 0);
    leds[5].setRGB(255, 0, 0);
    FastLED.show();
    onL();
  }
}




// ***************************************
// ************* Da Loop *****************
// ***************************************


void loop() {

  if (greenMode == 1) {
    while (greenMode == 1) {
      //waiting
    }
  }

  else {
    checkVol();
    checkRotaryButton();

    if (partyMode == 1) {
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */
      FillLEDsFromPaletteColors(startIndex);
      FastLED.show();
      FastLED.delay(1000 / UPDATES_PER_SECOND);
    }

    else {
      checkOpenDoor();
      spielSelector();
      indicatorPaint();
      rideStart();
    }
  }
  delay(10);
}




// ***************************************
// ********** Backbone Methods ***********
// ***************************************


void rideStart() {

  int bigRead = digitalRead(button);
  delay(10);
  while (digitalRead(button) == 0) {
    //waiting for release
  }

  if (bigRead == 0) {

    delay(100);
    int sTime = millis();

    //** Ride Stop
    if (spiel == 1) {
      // Starting the ride, you better vote!!
      onL();
      mp3_play_physical(1);
      // Did the ride stop?
      chanceTime = random(1, 3);
      if (chanceTime != 3) {  // Nope, we're good.
        failChance++;
      }
      if (chanceTime == 3 || failChance > 1) { //Oh uh, someone forgot to vote...
        offL();
        checkVol();
        delay(3600);

        mp3_play_physical(11);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        onL();
        checkVol();
        delay(1000);
        offL();
        checkVol();
        delay(1000);
        failChance = 0;
        checkVol();
      }
      else {
        delay(6000);
        offL();
        delay(100);
      }
    }

    //** Buss Bar   (too loud)
    else if (spiel == 2) {
      onL();
      mp3_play_physical(2);
      delay(15000);
      offL();
      delay(100);
    }

    //** Limited Capacity
    else if (spiel == 3) {
      onL();
      mp3_play_physical(3);
      delay(21000);
      offL();
      delay(1000);
    }

    //** Ride Start + SAE Retraction
    else if (spiel == 4) {
      onL();
      mp3_play_physical(1);
      delay(6500);
      mp3_play_physical(4);
      delay(16500);
      offL();
      delay(600);
    }

    //** Invisibillity Cloak Secret Queue Audio       slowly glowing
    else if (spiel == 5) {

      mp3_play_physical(5);

      int j = 0;
      int jDir = 1;
      Serial.println(sTime);
      Serial.println(millis());
      sTime = millis();
      int t = millis();
      for (; t - sTime < 22500; ) {
        if (jDir == 1) {
          j++;
          if (j == 255) {
            jDir = 0;
          }
        }
        else if (jDir == 0) {
          j--;
          if (j == 0) {
            jDir = 1;
          }
        }
        Serial.println(j);
        analogWrite(light, j);
        t = millis();
      }
      digitalWrite(light, LOW);
    }

    //** Hagrid Secret Queue Audio   just a little too loud     slowly glowing
    else if (spiel == 6) {

      mp3_play_physical(6);

      int j = 0;
      int jDir = 1;
      Serial.println(sTime);
      Serial.println(millis());
      sTime = millis();
      int t = millis();
      for (; t - sTime < 16000; ) {
        if (jDir == 1) {
          j++;
          if (j == 255) {
            jDir = 0;
          }
        }
        else if (jDir == 0) {
          j--;
          if (j == 0) {
            jDir = 1;
          }
        }
        Serial.println(j);
        analogWrite(light, j);
        t = millis();
      }
      digitalWrite(light, LOW);
    }


    //** "The Forest Isn't Safe"    add flash that dims on arana eximey
    else if (spiel == 7) {
      digitalWrite(light, LOW);
      mp3_play_physical(7);
      delay(6300);

      int j = 255;
      Serial.println(sTime);
      Serial.println(millis());
      sTime = millis();
      int t = millis();
      for (; t - sTime < 1000; ) {
        j -= 2;
        if (j <= 0) {
          break;
        }

        Serial.println(j);
        analogWrite(light, j);
        t = millis();
      }
      digitalWrite(light, LOW);
    }

    //** "1, 2, 3, Observatory!"    add light on 1, 2, 3, dim out after
    else if (spiel == 8) {
      digitalWrite(light, LOW);
      mp3_play_physical(8);
      delay(7000);

      int j = 0;
      Serial.println(sTime);
      Serial.println(millis());
      sTime = millis();
      int t = millis();
      for (; t - sTime < 1000; ) {
        j += 2;
        if (j >= 255) {
          break;
        }

        Serial.println(j);
        analogWrite(light, j);
        t = millis();
      }

      delay(200);
      j = 255;
      Serial.println(sTime);
      Serial.println(millis());
      sTime = millis();
      t = millis();
      for (; t - sTime < 1000; ) {
        j -= 2;
        if (j <= 0) {
          break;
        }

        Serial.println(j);
        analogWrite(light, j);
        t = millis();
      }

      digitalWrite(light, LOW);
    }

    //** Zelda Chest Opening
    else if (spiel == 9) {
      int count = 0;
      offL();
      mp3_play_physical(9);
      delay(200);
      while (count < 8) {
        onL();
        delay(170);
        offL();
        delay(325);
        count++;
      }
      delay(800);
      onL();
      delay(100);
      offL();
      delay(200);
      onL();
      delay(100);
      offL();
      delay(200);
      onL();
      delay(100);
      offL();
      delay(200);
      onL();
      delay(2050);
      offL();
      delay(2000);
    }

    //** Unlimited Power!
    else if (spiel == 10) {
      int count = 0;
      mp3_play_physical(10);
      offL();
      delay(3600);
      while (count < 46) {
        onL();
        delay(40);
        offL();
        delay(40);
        count++;
      }
      delay(2000);
    }

    //** Schiwvles
    else if (spiel == 11) {
      mp3_play_physical(12);    //songs 12 - whatever
      onL();
      delay(4000);
      offL();
      delay(100);
    }

    //** HP Meme Machine
    else if (spiel == 12) {
      onL();
      switch (memeCounter) {
        case 1:  // Cornish Pixies
          mp3_play_physical(13);
          delay(2000);
          break;
        case 2:  // Freshly Caught
          mp3_play_physical(14);
          delay(3000);
          break;
        case 3:  // Well Done Draco
          mp3_play_physical(15);
          delay(12000);
          break;
        case 4:  // Poof!
          mp3_play_physical(16);
          delay(6000);
          break;
        case 5:  // Something wrong with the pup
          mp3_play_physical(17);
          delay(5000);
          break;
        case 6:  // RIPPAH
          mp3_play_physical(18);
          delay(4000);
          break;
        case 7:  // Ahhw
          mp3_play_physical(19);
          delay(3000);
          break;
        case 8:  // Well done Slytherin
          mp3_play_physical(20);
          delay(5000);
          break;
        case 9:  // Mundungus
          mp3_play_physical(21);
          delay(2000);
          break;
        case 10:  // Nip it Mungungus
          mp3_play_physical(22);
          delay(2000);
          break;
        case 11:  // Good Luck!
          mp3_play_physical(23);
          delay(1200);
          break;
      }
      memeCounter++;
      if (memeCounter > 11) {
        memeCounter = 1;
      }
    }
    startPeriod = millis();
  }

  else {

    int timeChunk = millis() - startPeriod;

    if (timeChunk > 1000) {
      startPeriod = millis();
      if (lightState == 0) {
        lightState = 1;
        onL();
      }
      else if (lightState == 1) {
        lightState = 0;
        offL();
      }
      Serial.print("LightState: ");
      Serial.println(lightState);
    }
  }

}

void onL() {
  digitalWrite(light, HIGH);
}

void offL() {
  digitalWrite(light, LOW);
}

void checkOpenDoor() {

  if (digitalRead(doorSensor) == HIGH) { //Open
    doorState = 1;

    if (lastDoorState != doorState) {
      Serial.println("Door Open.");
    }
    leds[2].setRGB(255, 255, 255);
    leds[3].setRGB(255, 255, 255);
    leds[4].setRGB(255, 255, 255);
    leds[5].setRGB(255, 255, 255);
    FastLED.show();
  }
  else if (digitalRead(doorSensor) == LOW) { //Closed
    doorState = 0;
    if (lastDoorState != doorState) {
      Serial.println("Door Closed.");
    }
    leds[2].setRGB(0, 0, 0);
    leds[3].setRGB(0, 0, 0);
    leds[4].setRGB(0, 0, 0);
    leds[5].setRGB(0, 0, 0);
    FastLED.show();
  }
  lastDoorState = doorState;
}

void checkRotaryButton() {

  int q = digitalRead(rotaryButton);
  delay(10);
  while (digitalRead(rotaryButton) == 0) {
    //waiting
  }
  if (q == 0) {
    Serial.println("RotaryBtn Pressed!");
    if (partyMode == 0) {
      onL();
      partyMode = 1;
      mp3_play_physical(25); // Hermione Mix
    }
    else if (partyMode == 1) {
      offL();
      partyMode = 0;
      mp3_stop();
      FastLED.clear();
    }
    Serial.print("PartyMode: ");
    Serial.println(partyMode);
  }
}

void checkVol() {

  currentStateCLK = digitalRead(rotaryClock);  // Read the current state of inputCLK
  if (volChangeOccured == 1) {
    volChangeOccured = 0;
  }
  else {
    if (currentStateCLK != previousStateCLK) {  // If the inputDT state is different than the inputCLK state then
      if (digitalRead(rotaryData) != currentStateCLK) {  // the encoder is rotating counterclockwise

        if (previousVol == previousVol - 1) {
        }
        else {
          volume++;
          if (volume > 30) {
            volume = 30;
          }
          mp3_set_volume (volume);

          FastLED.clear();
          FastLED.show();
          int volBrightness = volume * 8.5;
          leds[1].setHSV(volBrightness, 255, 255);
          FastLED.show();
          delay(200);
          FastLED.clear();
          FastLED.show();
          delay(200);
          indicatorPaint();
        }
      }
      else { // Encoder is rotating clockwise

        if (previousVol == previousVol + 1) {
        }
        else {
          volume--;
          if (volume < 0) {
            volume = 0;
          }
          mp3_set_volume (volume);

          FastLED.clear();
          FastLED.show();
          int volBrightness = volume * 8.5;
          leds[0].setHSV(volBrightness, 255, 255);
          FastLED.show();
          delay(200);
          FastLED.clear();
          FastLED.show();
          delay(200);
          indicatorPaint();
        }
      }
      Serial.print("Volume: ");
      Serial.println(volume);
      volChangeOccured = 1;
    }
    previousPreviousVol = previousVol;
    previousVol = volume;
  }
  previousStateCLK = currentStateCLK;  // Update previousStateCLK with the current state
}



/**
   Colors in order: 
   blue     purple
   green    orange
   red      cyan
            yellow
 **/
void indicatorPaint() {

  if (spiel == 1) {
    leds[0].setRGB(0, 0, 30);
    leds[1].setRGB(21, 0, 32);
    FastLED.show();
  }
  else if (spiel == 2) {
    leds[0].setRGB(0, 0, 30);
    leds[1].setRGB(23, 23, 0);
    FastLED.show();
  }
  else if (spiel == 3) {
    leds[0].setRGB(0, 0, 30);
    leds[1].setRGB(23, 23, 0);
    FastLED.show();
  }
  else if (spiel == 4) {
    leds[0].setRGB(0, 0, 30);
    leds[1].setRGB(25, 20, 20);
    FastLED.show();
  }

  else if (spiel == 5) {
    leds[0].setRGB(0, 30, 0);
    leds[1].setRGB(21, 0, 32);
    FastLED.show();
  }
  else if (spiel == 6) {
    leds[0].setRGB(0, 30, 0);
    leds[1].setRGB(23, 20, 0);
    FastLED.show();
  }
  else if (spiel == 7) {
    leds[0].setRGB(0, 30, 0);
    leds[1].setRGB(23, 23, 0);
    FastLED.show();
  }
  else if (spiel == 8) {
    leds[0].setRGB(0, 30, 0);
    leds[1].setRGB(25, 20, 20);
    FastLED.show();
  }

  else if (spiel == 9) {
    leds[0].setRGB(28, 0, 0);
    leds[1].setRGB(21, 0, 32);
    FastLED.show();
  }
  else if (spiel == 10) {
    leds[0].setRGB(28, 0, 0);
    leds[1].setRGB(23, 20, 0);
    FastLED.show();
  }
  else if (spiel == 11) {
    leds[0].setRGB(28, 0, 0);
    leds[1].setRGB(23, 23, 0);
    FastLED.show();
  }
  else if (spiel == 12) {
    leds[0].setRGB(28, 0, 0);
    leds[1].setRGB(25, 20, 20);
    FastLED.show();
  }
}

void spielSelector() {

  int a = digitalRead(chooserUp);
  int b = digitalRead(chooserDown);
  int c = digitalRead(switchL);
  int d = digitalRead(switchR);
  delay(10);

  //Neutral State
  if ((a == 1) && (b == 1)) {
    if ((c == 1) && (d == 1)) { //Both Down
      spiel = 1;
    }
    else if ((c == 0) && (d == 1)) { //Left Up
      spiel = 2;
    }
    else if ((c == 1) && (d == 0)) { //Right Up
      spiel = 3;
    }
    else if ((c == 0) && (d == 0)) { //Both Up
      spiel = 4;
    }
  }

  //Up State
  else if ((a == 0) && (b == 1)) {
    if ((c == 1) && (d == 1)) { //Both Down
      spiel = 5;
    }
    else if ((c == 0) && (d == 1)) { //Left Up
      spiel = 6;
    }
    else if ((c == 1) && (d == 0)) { //Right Up
      spiel = 7;
    }
    else if ((c == 0) && (d == 0)) { //Both Up
      spiel = 8;
    }
  }

  //Down State
  else if ((a == 1) && (b == 0)) {
    if ((c == 1) && (d == 1)) { //Both Down
      spiel = 9;
    }
    else if ((c == 0) && (d == 1)) { //Left Up
      spiel = 10;
    }
    else if ((c == 1) && (d == 0)) { //Right Up
      spiel = 11;
    }
    else if ((c == 0) && (d == 0)) { //Both Up
      spiel = 12;
    }
  }
  if (spiel != lastSpiel) {
    Serial.print("Spiel: ");
    Serial.println(spiel);
  }
  lastSpiel = spiel;
}

void FillLEDsFromPaletteColors( uint8_t colorIndex) {

  for ( int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette( RainbowColors_p, colorIndex, 255, LINEARBLEND);
    colorIndex += 100;
  }
}
