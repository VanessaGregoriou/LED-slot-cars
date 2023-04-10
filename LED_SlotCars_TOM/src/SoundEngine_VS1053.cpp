
# include "SoundEngine_VS1053.hpp"

SoundEngine_VS1053::SoundEngine_VS1053(){
    
    musicPlayer =  new Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
}

void SoundEngine_VS1053::begin(void){

    if (! musicPlayer->begin()) { 
      Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
      while (1);
    }
    Serial.println(F("VS1053 found"));

    if (!SD.begin(CARDCS)) {
      Serial.println(F("SD failed, or not present"));
      while (1);  
    }
    Serial.println(F("SD found"));

    if(musicPlayer->useInterrupt(VS1053_FILEPLAYER_PIN_INT)){
      Serial.println(F("interupt working"));
    };  
    
    // make as loud as possible
    musicPlayer->setVolume(0,0);
}

void SoundEngine_VS1053::playSoundWithIndex(int i){

  char str[] = "/track00";
  char num[] = "1234";
  itoa(i,num,10);
  strcat(str, num);    
  strcat(str, ".mp3");    
  Serial.println(str);
  // musicPlayer->playFullFile(str);
    musicPlayer->startPlayingFile(str);


    // // char str[] = "/track00";
    // // char num[] = "00";
    // // itoa(i,num,10);

    // // strcat(str, num);    
    // // strcat(str, ".mp3");    
    // // Serial.println(str);

    // // musicPlayer->stopPlaying(); // must call stop before playing again!
    // musicPlayer->startPlayingFile(str);
}


