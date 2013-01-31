/*________________________________________________
 *|                                               \
 *|                                                \
 *|   D S 3 2 3 4   W O R D   C L O C K  - V2.0     \
 *|                                                  \
 *|                                                   \
 *|--------------------------------------------------|*
 *| COPYRIGHT   : (C)2012 Peter McKinney @           |*
 *|-------------: peter@pmck.info                    |*
 *|--------------------------------------------------|*
 *| AUTHOR      : Peter McKinney                     |*
 *|--------------------------------------------------|*
 *| REFRENCES   : Roughly bassed on drj113's code @  |*
 *|-------------: http://tinyurl.com/yg3jw94         |*
 *|-------------: and scottbez1's design and code @  |*
 *|-------------: http://tinyurl.com/7luakxx         |*
 *|--------------------------------------------------|*
 *| DESCRIPTION : An Arduino based clock using leds  |*
 *|-------------: behind a printed letter mask to    |*
 *|-------------: display the time using words.      |*
 *|-------------: uses a slightly modified           |*
 *|-------------: maniacbug DS3234 RTClib found      |*
 *|-------------: here  @                            |*_________
 *|-------------: http://github.com/maniacbug/RTClib |*         \
 *|-------------: updated version @                  |*          \
 *|-------------: http://github.com/pmck91/RTClib    |*           \
 *|--------------------------------------------------|*            \ 
 *| HARDWARE    : For list go to                     |*             \
 *|-------------: http://pmck.info/fancy-word-clock/ |*              \
 *|--------------------------------------------------|*               \
 *| TUTORIAL    : For build instructions go to       |*                \
 *|-------------: http://pmck.info/fancy-word-clock/ |*                 \
 *|--------------------------------------------------|*                  \
 *|                   __LICENCE__                    |*                   \
 *|----------------------------------------------------------------------|*
 *|This program is free software; you can redistribute it and/or modify  |*
 *|it under the terms of the GNU General Public License as published by  |*
 *| the Free Software Foundation; either version 2 of the License, or    |*
 *| (at your option) any later version.                                  |*
 *|                                                                      |*
 *| This program is distributed in the hope that it will be useful,      |*
 *| but WITHOUT ANY WARRANTY; without even the implied warranty of       |*
 *| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        |*
 *| GNU General Public License for more details.                         |*
 *|                                                                      |*
 *| You should have received a copy of the GNU General Public License    |*
 *| along with this program; if not, write to the Free Software          |*
 *| Foundation, Inc., 59 Temple Place, Suite 330, Boston.                |*
 *| Please see LICENCE file for full licence                             |*
 *|----------------------------------------------------------------------|*/ 
 
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <RTC_DS3234.h>

// confused by whats in the following defines? this should help -> http://tinyurl.com/glb6p
//--------- MINUTE DISPLAY CONTROL --------//
#define MTEN 	Display1=Display1 | (1<<0)  
#define HALF	Display1=Display1 | (1<<1)
#define QUARTER	Display1=Display1 | (1<<2)
#define TWENTY	Display1=Display1 | (1<<3)
#define MFIVE	Display1=Display1 | (1<<4)
#define MINUTES	Display1=Display1 | (1<<5)
#define PAST	Display1=Display1 | (1<<6)
#define A	Display1=Display1 | (1<<7)

//----------- HOUR DISPLAY ONE ------------//
#define TO	Display2=Display2 | (1<<0)
#define ONE	Display2=Display2 | (1<<1)
#define TWO	Display2=Display2 | (1<<2)
#define THREE	Display2=Display2 | (1<<3)
#define FOUR	Display2=Display2 | (1<<4)
#define HFIVE	Display2=Display2 | (1<<5)
#define SIX	Display2=Display2 | (1<<6)
#define UNUSED2	Display2=Display2 | (1<<7)

//------------ HOUR DISPLAY TWO ------------//
#define SEVEN	Display3=Display3 | (1<<0)
#define EIGHT	Display3=Display3 | (1<<1)
#define NINE	Display3=Display3 | (1<<2)
#define HTEN	Display3=Display3 | (1<<3)
#define ELEVEN	Display3=Display3 | (1<<4)
#define TWELVE	Display3=Display3 | (1<<5)
#define OCLOCK  Display3=Display3 | (1<<6)
#define UNUSED3	Display3=Display3 | (1<<7)

// SHIFT REGISTER PINS
#define LCP 6 // led clock pin
#define LDP 7 // led data pin
#define LSP 8 // led strobe pin

// BUTTON PINS
#define MBP 2 // minute button
#define HBP 3 // hour button
#define DSP 4 // daylight savings

// BRIGHTNESS PINS
#define BSP 9  // brightness output pin

// RTC DATA
RTC_DS3234 RTC(10); // RTC class

uint16_t year; 
uint8_t month;
uint8_t day;
uint8_t hour;
uint8_t minute;
uint8_t second;

DateTime timenow;

// GENERAL DATA + VARIABLES 
boolean dayLightSavings = false; // +1hour if true 
boolean testMode = false;
long previousMillisPrint = 0;
long previousMillisButtonPress = 0;
long printInterval = 1000; 
long buttonInterval = 333; 

char Display1=0, Display2=0, Display3=0; // the three display shifters each 8 bits long


void setup(){
  
  // sets up the output pins
  pinMode(LCP, OUTPUT); 
  pinMode(LDP, OUTPUT); 
  pinMode(LSP, OUTPUT);
  pinMode(BSP, OUTPUT); 
  
  // sets up the button pins
  pinMode(MBP , INPUT);
  pinMode(HBP , INPUT);
  pinMode(DSP , INPUT);
  
  digitalWrite(MBP, HIGH);
  digitalWrite(HBP, HIGH);
  digitalWrite(DSP, HIGH);

  Serial.begin(9600);
  Serial.println("Starting monitoring");
  
  SPI.begin();
  RTC.begin();
  
} // END OF SETUP


void loop(){
  
  if(testMode){
    displayTest();
    delay(1000);
  }
  
  else{
    
    unsigned long currentMillis = millis();

    if(currentMillis - previousMillisPrint > printInterval) {
      previousMillisPrint = currentMillis; 
      serialPrintTime();
    } 
    
    analogWrite(BSP, 255);
    
    displaytime(); 
    
    if(digitalRead(MBP) ==0 && digitalRead(HBP)!=0 && digitalRead(DSP)!=0 ) {
      
      if(currentMillis - previousMillisButtonPress > buttonInterval) {
        previousMillisButtonPress = currentMillis;
        Serial.println("minute ++");
    
        //add one minute
        year = timenow.year();
        month = timenow.month(); 
        day = timenow.day();
        hour = timenow.hour(); 
        minute = timenow.minute();
        second = 0;
        
        if(++minute >= 60){
          minute = 0;
        }
        
        DateTime timeToSet(year,month,day,hour,minute,second);
        
        RTC.adjust(timeToSet);
        
        serialPrintTime();
        
        displaytime();
      }
    }
    
    if(digitalRead(HBP)==0 && digitalRead(MBP)!=0 && digitalRead(DSP)!=0 ){
      
      if(currentMillis - previousMillisButtonPress > buttonInterval) {
        previousMillisButtonPress = currentMillis;
        Serial.println("Hour ++");
        
        // add one hour
        year = timenow.year();
        month = timenow.month(); 
        day = timenow.day();
        hour = timenow.hour(); 
        minute = timenow.minute();
        second = 0;
        
        if(++hour >= 24){
          hour = 0;
        }
        
        DateTime timeToSet(year,month,day,hour,minute,second);
        
        RTC.adjust(timeToSet);
        
        serialPrintTime();
        
        displaytime();
      }
    }
    
    if(digitalRead(DSP)==0 && digitalRead(MBP)!=0 && digitalRead(HBP)!=0 ){
      
      if(currentMillis - previousMillisButtonPress > buttonInterval) {
        previousMillisButtonPress = currentMillis;
      
        Serial.print("Daylight savigns are enabled (plus one hour if yes)?  ->  ");
        Serial.println(dayLightSavings);
        
        // add or remove one hour based on daylight savings
        year = timenow.year();
        month = timenow.month(); 
        day = timenow.day();
        hour = timenow.hour(); 
        minute = timenow.minute();
        second = 0;
        
        dayLightSavings = !dayLightSavings;
        
        if(dayLightSavings){
            if(++hour == 24){
              hour = 0;
            }
        }
        else{
          if(hour-- == 0){
              hour = 23;
          }
        }
        
        DateTime timeToSet(year,month,day,hour,minute,second);
        
        RTC.adjust(timeToSet);
        
        serialPrintTime();
        
        displaytime();
      }
    }
  
  }
  
  if (digitalRead(HBP)==0 && digitalRead(MBP)==0 ){
   testMode = !testMode;
   delay(5000); // stop user entering test mode straight after exiting it 
  
  }

} // END OF LOOP


void ledsoff(void) {
  
  // zero the displays bits
  Display1=0;
  Display2=0;
  Display3=0;
  
} // END OF LEDS OFF


void WriteLEDs(void) {
  
  // shift out display 1 to 3
  shiftOut(LDP, LCP, MSBFIRST, Display3);
  shiftOut(LDP, LCP, MSBFIRST, Display2);
  shiftOut(LDP, LCP, MSBFIRST, Display1);
  digitalWrite(LSP,HIGH);
  delay(2);
  digitalWrite(LSP,LOW); 
  
} // END OF WRITELEDS


void displayTest(void){
  
  //runs a test on the display to check all LEDs are working
  Serial.print("\nTEST\n");
  analogWrite(BSP, 255);
  
  ledsoff(); MTEN; WriteLEDs(); delay(250); 
  ledsoff(); HALF; A; WriteLEDs(); delay(250); 
  ledsoff(); A; QUARTER; WriteLEDs(); delay(250); 
  ledsoff(); TWENTY; WriteLEDs(); delay(250); 
  ledsoff(); MFIVE; WriteLEDs(); delay(250); 
  ledsoff(); MINUTES; WriteLEDs(); delay(250); 
  ledsoff(); PAST; WriteLEDs(); delay(250); 
  ledsoff(); TO; WriteLEDs(); delay(250); 
  ledsoff(); ONE; WriteLEDs(); delay(250); 
  ledsoff(); TWO; WriteLEDs(); delay(250); 
  ledsoff(); THREE; WriteLEDs(); delay(250); 
  ledsoff(); FOUR; WriteLEDs(); delay(250); 
  ledsoff(); HFIVE; WriteLEDs(); delay(250); 
  ledsoff(); SIX; WriteLEDs(); delay(250); 
  ledsoff(); SEVEN; WriteLEDs(); delay(250); 
  ledsoff(); EIGHT; WriteLEDs(); delay(250); 
  ledsoff(); NINE; WriteLEDs(); delay(250); 
  ledsoff(); HTEN; WriteLEDs(); delay(250); 
  ledsoff(); ELEVEN; WriteLEDs(); delay(250); 
  ledsoff(); TWELVE; WriteLEDs(); delay(250); 
  ledsoff(); OCLOCK; WriteLEDs(); delay(250);
  ledsoff(); 
  
  analogWrite(BSP, 255);
  
  MTEN; HALF; A; QUARTER; TWENTY; MFIVE;
  MINUTES; PAST; TO; ONE; TWO; THREE;
  FOUR; HFIVE; SIX; SEVEN; EIGHT; NINE; 
  HTEN; ELEVEN; TWELVE; OCLOCK; 
  
  WriteLEDs(); 
  
  for(int i = 255; i >= 0; i--){
    analogWrite(BSP,i);
    delay(40);
  }
  for(int i = 0; i < 255; i++){
    analogWrite(BSP,i);
    delay(40);
  }
  
  ledsoff();
  
} // END OF DISPLAYTEST


void displaytime(void){
  
  // turn off all LEDs
  ledsoff();


// display the appropriate minute counter
  if ((timenow.minute()>4) && (timenow.minute()<10)) { 
    MFIVE; 
    MINUTES; 
  } 
  if ((timenow.minute()>9) && (timenow.minute()<15)) { 
    MTEN; 
    MINUTES; 
  }
  if ((timenow.minute()>14) && (timenow.minute()<20)) {
    A; QUARTER; 
  }
  if ((timenow.minute()>19) && (timenow.minute()<25)) { 
    TWENTY; 
    MINUTES; 
  }
  if ((timenow.minute()>24) && (timenow.minute()<30)) { 
    TWENTY; 
    MFIVE; 
    MINUTES;
  }  
  if ((timenow.minute()>29) && (timenow.minute()<35)) {
    HALF; A;
  }
  if ((timenow.minute()>34) && (timenow.minute()<40)) { 
    TWENTY; 
    MFIVE; 
    MINUTES;
  }  
  if ((timenow.minute()>39) && (timenow.minute()<45)) { 
    TWENTY; 
    MINUTES; 
  }
  if ((timenow.minute()>44) && (timenow.minute()<50)) {
    A; QUARTER; 
  }
  if ((timenow.minute()>49) && (timenow.minute()<55)) { 
    MTEN; 
    MINUTES; 
  } 
  if (timenow.minute()>54) { 
    MFIVE; 
    MINUTES; 
  }

  // display the correct hour depending on how close we are to the next or last
  if ((timenow.minute() <5)){
    
    switch (timenow.hour()) {
      case 1:
      case 13:	
        ONE; 
        break;
      case 2: 
      case 14:
        TWO; 
        break;
      case 3: 
      case 15:
        THREE; 
        break;
      case 4: 
      case 16:
        FOUR; 
        break;
      case 5: 
      case 17:
        HFIVE; 
        break;
      case 6: 
      case 18:
        SIX; 
        break;
      case 7: 
      case 19:
        SEVEN; 
        break;
      case 8: 
      case 20:
        EIGHT; 
        break;
      case 9: 
      case 21:
        NINE; 
        break;
      case 10:
      case 22:	
        HTEN; 
        break;
      case 11:
      case 23:	
        ELEVEN; 
        break;
      case 12:
      case 0:	
        TWELVE; 
        break;
    }
    OCLOCK;
  }
  
  else if ((timenow.minute() < 35) && (timenow.minute() >4)){
    
    PAST;
    switch (timenow.hour()) {
      case 1:
      case 13: 
        ONE; 
        break;
      case 2:
      case 14: 
        TWO; 
        break;
      case 3:
      case 15: 
        THREE; 
        break;
      case 4:
      case 16: 
        FOUR; 
        break;
      case 5:
      case 17: 
        HFIVE; 
        break;
      case 6:
      case 18: 
        SIX; 
        break;
      case 7:
      case 19: 
        SEVEN; 
        break;
      case 8:
      case 20: 
        EIGHT; 
        break;
      case 9:
      case 21: 
        NINE; 
        break;
      case 10:
      case 22: 
        HTEN; 
        break;
      case 11:
      case 23: 
        ELEVEN; 
        break;
      case 12:
      case 0: 
        TWELVE; 
        break;
    }
  }
  
  else{
    
    TO;
    switch (timenow.hour()) {
      case 1: 
      case 13:
        TWO; 
        break;
      case 2: 
      case 14:
        THREE; 
        break;
      case 3: 
      case 15:
        FOUR; 
        break;
      case 4: 
      case 16:
        HFIVE; 
        break;
      case 5: 
      case 17:
        SIX; 
        break;
      case 6: 
      case 18:
        SEVEN; 
        break;
      case 7: 
      case 19:
        EIGHT; 
        break;
      case 8: 
      case 20:
        NINE; 
        break;
      case 9: 
      case 21:
        HTEN; 
        break;
      case 10:
      case 22:	  
        ELEVEN; 
        break;
      case 11:
      case 23:
        TWELVE; 
        break;
      case 12:
      case 0:	  
        ONE; 
        break;
    }
  }

  WriteLEDs();
  
} // END OF DISPLAYTIME

void serialPrintTime(void){
  
  timenow = RTC.now();
  Serial.print(timenow.hour()); 
  Serial.print(":"); 
  Serial.print(timenow.minute());  
  Serial.print(":"); 
  Serial.print(timenow.second()); 
  Serial.print(" - "); 
  Serial.print(timenow.day()); 
  Serial.print("/"); 
  Serial.print(timenow.month()); 
  Serial.print("/"); 
  Serial.print(timenow.year());
  Serial.println();     
  
} // END OF SERIALPRINTTIME
