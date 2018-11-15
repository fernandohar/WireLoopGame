 #include <Arduino.h>
#include "SoftwareSerial.h"
#include "MP3TF16P.h"




///////////////////////MP3 END//////////////////////////
#define MAXFILE 2
SoftwareSerial ss(10,11);
MP3TF16P mp3(&ss, &Serial);
int mp3FileName2Play = 0;
#define MP3_CHECK_FREQ 1500
unsigned long mp3LastCheck = 0;
unsigned long songStartTime;
///////////////////////mp3 END//////////////////////////

//Debounce handling
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 20;
int lastButtonState = LOW;
int buttonState = LOW;

unsigned long gameoverTime = 0;
unsigned long gameoverMusicDelay = 150;

//GAME logic ///
#define MAXLIFE 5
#define GAMEPIN 2  


int life = 0;

void updateLifeLED(){
  Serial.print("updateLifeLED, Life:[");
  Serial.print(life);
  Serial.println("]");
	//PORTD: PIN# 0 - 7
	byte mask = 0;
	for (int i = 1; i <= life; ++i){
		mask = mask << 1;
		mask = mask | 1; //	
	}
	mask = mask << 3;
	mask |= B00000111; //do not update D2, Rx, Tx
//  Serial.print("mask");
//  Serial.println(mask, BIN);
  PORTD &= B00000111;
	PORTD |= mask;	
  //digitalWrite(7, HIGH);
}

void initLifeLED(){
	// set pins 1 (serial transmit) and 3..7 as output,
	// pin 0 (serial receive) & pin 2 as input
	DDRD = B11111010;  // digital pins 7,6,5,4,3,2,1,0
}

void resetGame(){
	life = MAXLIFE;
	updateLifeLED();
	//Start Loop background sound
 Serial.println("RepeatPlay");
  mp3.repeatPlay(1);
  //mp3.playRandom();
}

void reduceLife(){
	--life;
//	Serial.print("Life: [");
//	Serial.print(life); 
//	Serial.println("]");
	
	updateLifeLED();
	if (life > 0){
		//interrupt and play hit sound
    mp3.playAdFile(2);
	}else{
		//Stop background sound;
    Serial.print("Game Over");
    mp3.stop();
    delay(50);
		//play Gameover sound
   mp3.playMp3File(3);
   gameoverTime = millis();
	}
	
}

void initMP3TF16P(){
    mp3.setDebug(false);
    mp3.begin();
    mp3.stop();
    delay(10);
    mp3.setAmplification(false, 0);
    mp3.setVol(20);
    delay(100);
    uint8_t mp3_Vol = mp3.getVol();
    Serial.print("Volume:"); Serial.println(mp3_Vol);
}
bool continueCheckWire = true;
bool checkWire(){
  //Handle Debounce of input pin
  int reading = digitalRead(GAMEPIN);
  if (reading != lastButtonState){
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay){
    if (reading != buttonState){
      buttonState = reading;
      if(buttonState == HIGH){
        reduceLife();
      }
    }
  }
  lastButtonState = reading;
}

void playSong(){
   uint8_t playStatus = mp3.getPlayStatus();
   Serial.print("PlayStatus");
   Serial.println(playStatus);
  if(playStatus == 8){
    Serial.println("[OFFLINE]");
    return;
  }else if (playStatus == 1){
    unsigned long elapse = (millis() - songStartTime) / 1000;
    Serial.print ("Elapse time:");
    Serial.println(elapse);
    return;
  }else if (playStatus == 2){
    mp3.play();
    Serial.println("[RESUME]");
  }else if (playStatus == 0){
   if(++mp3FileName2Play > MAXFILE){
      mp3FileName2Play = 1;
    }
    mp3.playMp3File(mp3FileName2Play);
    songStartTime = millis();
    Serial.print("playMp3File #");
    Serial.println(mp3FileName2Play);
  }else{
    Serial.print("[ERROR]");
    Serial.println(playStatus);
  }
}
void stopSong(){
  mp3.pause();

  Serial.println("PAUSED");
}

void setup() {
    Serial.begin(115200);
    Serial.println("Wire Game Loop Initializing");
    //Hardware initialization

	initLifeLED();
    initMP3TF16P();
	resetGame();
	Serial.println("Initialization completed");
}

void loop() {
	if (life > 0){
		checkWire();
	}else{
    if( (millis() - gameoverTime) > gameoverMusicDelay){
      uint8_t playStatus = mp3.getPlayStatus();
      Serial.print("PlayStatus");
      Serial.println(playStatus);
      if(playStatus == 0){
        resetGame();
      }
    }
	}
}
