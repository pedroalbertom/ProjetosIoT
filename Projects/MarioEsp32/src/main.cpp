#include <Arduino.h>
#include <Mario.h>

int buzzerPin = 13;

void tocarMusica(u_int16_t musica[], uint8_t duration[], uint8_t size){
    for(int i = 0; i < size; i++){
        int noteDuration = 1000 / duration[i];
        
        if(musica[i] > 30 && musica[i] < 4979){
            tone(buzzerPin, musica[i], noteDuration);
        }else{
            noTone(buzzerPin);
        }
        delay(noteDuration);
    }    
}

void setup(){
    pinMode(buzzerPin, OUTPUT);
}

void loop(){
    tocarMusica(mario, marioDurations, sizeMario);
    delay(1000);
}
