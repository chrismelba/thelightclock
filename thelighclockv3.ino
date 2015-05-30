#include <Wire.h> 
#include "RTClib.h" 
#include <avr/power.h>
#include <FastLED.h>



#define PIXEL_PIN 6  //the pin that attaches to the neopixel strip
#define N_PIX 100 //the number of pixels in the strip
#define colourbuttonPin 9     // the number of the button to change colour
#define modebuttonPin 10 // the number of the button to change mode


RTC_DS1307 rtc;

DateTime now;


CHSV Hour_Colour(64,255,255);
CHSV Minute_Colour(160,255,255);
CHSV Second_Colour(0, 0, 255);

// CRGB Hour_Colour= CRGB::Yellow;
// CRGB Minute_Colour= CRGB::Blue;
// CRGB Second_Colour= CRGB::White;
int Secondsonoff = 1;
int faceMode = 0;
int colourMode = 0; 
int Randommode;
int debug = 0;
int testrun = 1;
int pressholdcheck1 = 0;

//Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
// NEO_KHZ800800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEO_GRB Pixels are wired for GRB bitstream (most NeoPixel products)

CRGB leds[N_PIX];



void setup() {
  // try fastled code??

  //FastLED.addLeds<NEOPIXEL, PIXEL_PIN>(leds, N_PIX);

  // initialize the pushbutton pin as an input:
  pinMode(modebuttonPin, INPUT_PULLUP); 
  pinMode(colourbuttonPin, INPUT_PULLUP); 


  Serial.begin(115200);
  Wire.begin();
  rtc.begin();

  FastLED.addLeds<NEOPIXEL, PIXEL_PIN>(leds, N_PIX);
  FastLED.setBrightness(50);

    //lower brightness for low voltage applications, like when powering off computer USB to test code. ENSURE YOU REMOVE THIS LINE PRIOR TO FINAL BUILD
    //strip.setBrightness(30);
    

    if (! rtc.isrunning()) {
    	Serial.println("RTC is NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  now = rtc.now();

}

void loop() {
      int secs;
  int buttonState;
  uint16_t hour_pos;
  uint16_t mins_pos;

    // I wanted a debug mode to run the face faster to test out colour combinations

  if(testrun == 1){
        // get the position of the hour hand and the minute hand
    hour_pos = (now.minute() % 12) * N_PIX / 12 + now.second()/6;
    mins_pos = now.second() * N_PIX / 60;

    } else {
          // get the position of the hour hand and the minute hand
      hour_pos = (now.hour() % 12) * N_PIX / 12 + now.minute()/6;
      mins_pos = now.minute() * N_PIX / 60;

  }
// call the face that has it's mode currently active
  switch (faceMode) {
    case 0:
    fadeface(hour_pos, mins_pos);
    break;
    case 1:
    chsvface(hour_pos, mins_pos);
    // break;
    // case 2:
    // fadeblockface(hour_pos, mins_pos);
    // break;
    // case 3:
    // aprilface(hour_pos, mins_pos);
    // break;
  }

  // if(Secondsonoff == 1 && testrun == 0){
  //  int sec_pos = now.second() * N_PIX / 60;
  //  strip.setPixelColor(sec_pos, Second_Colour[0], Second_Colour[1], Second_Colour[2]);

  // }
  FastLED.show();

  buttonState = digitalRead(modebuttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is LOW:
  if (buttonState == LOW) {     

    //check if the button has been continuously held down since it was pressed.
    if(pressholdcheck1 == 0) {
      faceMode++;
      faceMode%=4;
      pressholdcheck1 = 1;
    }
  } 
  else {

    pressholdcheck1 = 0;
  }

  secs = now.second();

  now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

}

void fadeface(uint16_t hour_pos, uint16_t min_pos){
	fract8 fadepoint;
	fract8 midpoint = 128;
	CHSV fadecolour(0,0,100);
	CHSV hourpix;
	CHSV minpix;
	CHSV blendedpix;


	for(int j=0; j<N_PIX; j++){


	fadepoint = (min_pos+j)%N_PIX*255;
	minpix = nblend(fadecolour, Minute_Colour,fadepoint);
	fadepoint = (hour_pos+j)%N_PIX*255;
	hourpix = nblend(fadecolour, Hour_Colour,fadepoint);

	blendedpix = blend(hourpix, minpix, midpoint);

	leds[N_PIX-j-1] = blendedpix;
	//Serial.println(addedpix[j]);

    }
}
void chsvface(uint16_t hour_pos, uint16_t min_pos) {
	fract8 fadepoint;
	fract8 midpoint = 128;
	CHSV hourpix;
	CHSV minpix;
	CHSV blendedpix;
	int minstr;
	int hourstr;




	for(int j=0; j<N_PIX; j++){
		// fadepoint = ((j+hour_pos)%N_PIX)*255/120;
	 //   hourpix = blend(Hour_Colour, fadecolour, fadepoint);
	 //    fadepoint = ((j+min_pos)%N_PIX)*255/120;
	 //    minpix = blend(Minute_Colour, fadecolour, fadepoint);
	 	hourstr = (N_PIX+j-hour_pos)%N_PIX;
	 	minstr = (N_PIX+j-min_pos)%N_PIX;
	    fadepoint= 255-255*hourstr/(hourstr+minstr);
	    blendedpix = blend(Hour_Colour, Minute_Colour, fadepoint);

	    leds[j] = blendedpix;
	            
    }

}
