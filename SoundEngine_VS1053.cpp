
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
    while (1);  // don't do anything more
    }

    // make as loud as possible
    musicPlayer->setVolume(0,0);

  if(musicPlayer->useInterrupt(VS1053_FILEPLAYER_PIN_INT)){
    Serial.print("interupt working");
  };  
}

void SoundEngine_VS1053::playSoundWithIndex(int i){

    String s = "/track00";
    s = s + i;
    s = s + ".mp3";
    musicPlayer->stopPlaying(); // must call stop before playing again!
    Serial.println(s.c_str());
    musicPlayer->startPlayingFile(s.c_str());
}


