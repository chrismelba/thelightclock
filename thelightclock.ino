#include <Wire.h> 
#include "RTClib.h" 
#include <avr/power.h>
#include "Adafruit_NeoPixel.h"
//#include <FastLED.h>



#define PIXEL_PIN 6 

int N_PIX=120;
RTC_DS1307 rtc;
DateTime now;
int fadecolour = 100;

int Hour_Colour[3] = {255, 230, 0};
int Hour_Colour2[3] = {fadecolour, fadecolour, fadecolour};
int Minute_Colour[3] = {0, 230, 255};
int Minute_Colour2[3] = {fadecolour, fadecolour, fadecolour};
int Second_Colour[3] = {255, 255, 255};
int Secondsonoff = 1;
int faceMode = 0;
int colourMode = 0; 
int Randommode;
int debug = 0;
int testrun = 0;
int pressholdcheck1 = 0;

//Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
// NEO_KHZ800800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEO_GRB Pixels are wired for GRB bitstream (most NeoPixel products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_PIX, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

//CRGB leds[120];


const int colourbuttonPin = 9;     // the number of the button to change colour
const int modebuttonPin = 10; // the number of the button to change mode



void setup() {
  // try fastled code??

  //FastLED.addLeds<NEOPIXEL, PIXEL_PIN>(leds, N_PIX);

  // initialize the pushbutton pin as an input:
  pinMode(modebuttonPin, INPUT_PULLUP); 
  pinMode(colourbuttonPin, INPUT_PULLUP); 


  Serial.begin(115200);
  Wire.begin();
  rtc.begin();

  strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    
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
    blockface(hour_pos, mins_pos);
    break;
    case 2:
    fadeblockface(hour_pos, mins_pos);
    break;
    case 3:
    aprilface(hour_pos, mins_pos);
    break;
  }

  if(Secondsonoff == 1 && testrun == 0){
   int sec_pos = now.second() * N_PIX / 60;
   strip.setPixelColor(sec_pos, Second_Colour[0], Second_Colour[1], Second_Colour[2]);

  }
  strip.show();

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



void fadeface(uint16_t hour_pos, uint16_t mins_pos) {
    int pix[3];
    int hourpix[3];
    int minpix[3];

    //loop around the clock face and determine the colour of each pixel
    for(int j=0; j<N_PIX; j++){

      interpolate_pix(Hour_Colour, Hour_Colour2, N_PIX, (hour_pos+j)%N_PIX, hourpix);
      interpolate_pix(Minute_Colour, Minute_Colour2, N_PIX, (mins_pos+j)%N_PIX, minpix);

      add_pix_cmyk(hourpix, minpix, pix);

      strip.setPixelColor(N_PIX-j-1,pix[0],pix[1],pix[2]);
        //Serial.println(addedpix[j]);
        
    }


}

void aprilface(uint16_t hour_pos, uint16_t mins_pos) {
    //this face will have two faded tails which add together like the standard face, but the tails will be much shorter
    int pix[3];
    int hourpix[3];
    int minpix[3];

    int fadesize = N_PIX/4;
    //loop around the clock face and determine the colour of each pixel
    for(int j=0; j<N_PIX; j++){

      interpolate_pix(Hour_Colour, Hour_Colour2, fadesize, (hour_pos+j)%fadesize, hourpix);
      if((j+hour_pos)%N_PIX>=fadesize){
        for(int i=0; i<3; i++){
            hourpix[i] = 0;
        }
        
      }

      interpolate_pix(Minute_Colour, Minute_Colour2, fadesize, (mins_pos+j)%fadesize, minpix);
      if((j+mins_pos)%N_PIX>=fadesize){
       for(int i=0; i<3; i++){
            minpix[i] = 0;
        }
        
      }


      averagergb(hourpix, minpix, pix);

      strip.setPixelColor(N_PIX-j-1,pix[0],pix[1],pix[2]);
        //Serial.println(addedpix[j]);
        
    }


}

    void blockface(uint16_t hour_pos, uint16_t mins_pos) {


      int c1[3];
      int c2[3];
      int firsthand = min(hour_pos, mins_pos);
      int secondhand = max(hour_pos, mins_pos);
    //check which hand is first, so we know what colour the 0 pixel is

    if(hour_pos>mins_pos){
      for(int i=0; i<3; i++){
        c1[i] = Hour_Colour[i];
        c2[i] = Minute_Colour[i];    
      }
      

    }
    else
    {
      for(int i=0; i<3; i++){
        c2[i] = Hour_Colour[i];
        c1[i] = Minute_Colour[i];    
      }
    }

    //loop the first colour until we hit the first hand
    for(int i=0; i<firsthand; i++){
      strip.setPixelColor(i,c1[0],c1[1],c1[2]);        
    }
    //loop the second colour to the second hand
    for(int i=firsthand; i<secondhand; i++){
      strip.setPixelColor(i,c2[0],c2[1],c2[2]);         
    }
    //and the last hand
    for(int i=secondhand; i<N_PIX; i++){
      strip.setPixelColor(i,c1[0],c1[1],c1[2]);        
    }

    strip.show();
  }

  void fadeblockface(uint16_t hour_pos, uint16_t mins_pos) {

    int c1[3];
    int c2[3];
    int c1fade[3];
    int c2fade[3];
    int firsthand = min(hour_pos, mins_pos);
    int secondhand = max(hour_pos, mins_pos);
    int pix[3];
    //check which hand is first, so we know what colour the 0 pixel is

    if(hour_pos>mins_pos){
      for(int i=0; i<3; i++){
        c1[i] = Hour_Colour[i];
        c1fade[i] = Hour_Colour2[i];

        c2[i] = Minute_Colour[i];    
        c2fade[i] = Minute_Colour2[i];    
      }
      

    }
    else
    {
      for(int i=0; i<3; i++){
        c2[i] = Hour_Colour[i];
        c2fade[i] = Hour_Colour2[i];

        c1[i] = Minute_Colour[i];    
        c1fade[i] = Minute_Colour2[i];    
      }
    }

    //loop the first colour until we hit the first hand
    for(int i=0; i<firsthand; i++){
      interpolate_pix(c1fade, c1, (N_PIX-secondhand+firsthand), (N_PIX-secondhand+i), pix);
      strip.setPixelColor(i,pix[0],pix[1],pix[2]);        
    }
    //loop the second colour to the second hand
    for(int i=firsthand; i<secondhand; i++){
      interpolate_pix(c2fade, c2, (secondhand-firsthand), (i-firsthand), pix);
      strip.setPixelColor(i,pix[0],pix[1],pix[2]);         
    }
    //and the last hand
    for(int i=secondhand; i<N_PIX; i++){
      interpolate_pix(c1fade, c1, secondhand+firsthand, i-secondhand, pix);
      strip.setPixelColor(i,pix[0],pix[1],pix[2]);        
    }

    strip.show();

  }


  void interpolate_pix(int rgb1[3], int rgb2[3], int arraysize, int pos, int output[3])
  {

    //this code will take two RGB colour inputs and output an interpolated point between them
    
    //calculate each colour of each pixel. We do the calculations one at a time otherwise integer maths ruins everything
    for(int j=0; j<3; j++){
      output[j] = rgb2[j]-rgb1[j];
      output[j] = pos * output[j];
      output[j] = output[j]/arraysize;
      output[j] = output[j] + rgb1[j];

    }

  }

  void add_pix_cmyk(int in1[3], int in2[3], int output[3]) {
//This function will take two pixels in RGB space and add them using cmyk method

int cmyk1[4];
int cmyk2[4];
int cmykadded[4];

// convert RGB to CMYK
rgb_to_cmyk(in1, cmyk1);
rgb_to_cmyk(in2, cmyk2);

//add the cmyk pix together
add_cmyk(cmyk1, cmyk2, cmykadded);

//convert back to rgb
cmyk_to_rgb(cmykadded, output);

}


void rgb_to_cmyk(const int rgb[], int cmyk[])
{
  uint16_t c, m, y, k;

  /* Special case all black */
  if (rgb[0] == 0 && rgb[1] == 0 &&
    rgb[2] == 0) {
    cmyk[0] = cmyk[1] = cmyk[2] = 0;
    cmyk[3] = 255;
    return;
  }

  /* Track K as the fademin of CMY as we go */
  k = c = 255 - rgb[0];
  m = 255 - rgb[1];
  if (m < k)
  k = m;
  y = 255 - rgb[2];
  if (y < k)
  k = y;

  /* Scale CMY to fill the full 0..255 range */
  c = (c - k) * 255 / (255 - k);
  m = (m - k) * 255 / (255 - k);
  y = (y - k) * 255 / (255 - k);

  cmyk[0] = c;
  cmyk[1] = m;
  cmyk[2] = y;
  cmyk[3] = k;
}

void cmyk_to_rgb(const int cmyk[], int rgb[])
{
  uint16_t c, m, y, k;

  /* Special case grey shades with pure K */
  if (cmyk[0] == 0 && cmyk[1] == 0 &&
    cmyk[2] == 0) {
    rgb[0] = rgb[1] = rgb[2] = cmyk[3];
    return;
  }

  k = 255 - cmyk[3];
  c = cmyk[0] * k / 255;
  m = cmyk[1] * k / 255;
  y = cmyk[2] * k / 255;
  k = cmyk[3];
  
  rgb[0] = 255 - (c + k);
  rgb[1] = 255 - (m + k);
  rgb[2] = 255 - (y + k);
}

void averagergb(const int in1[], const int in2[], int out[]) {

  for(int i=0; i<3; i++){
    out[i]=(in1[i]+in2[i])/2;
      
  }
}

void add_cmyk (const int in1[], const int in2[], int out[])
{
  int i;
  uint16_t tmp[4];
  uint16_t fademin_cmy = 255;

  /* Blend CMYK by addition, then
  * normalise CMY into K */
  for (i = 0; i < 4; i++) {
    tmp[i]  = in1[i] + in2[i];
    if (tmp[i] < fademin_cmy)
    fademin_cmy = tmp[i];
  }
  for (i = 0; i < 3; i++) {
    if (tmp[i] - fademin_cmy > 255)
    out[i] = 255;
    else
    out[i] = tmp[i] - fademin_cmy;
  }
  if (tmp[3] + fademin_cmy > 255)
  out[3] = 255;
  else
  out[3] = tmp[3] + fademin_cmy;
}

void add_cmyk_normalise (const int in1[], const int in2[], int out[])
{
  int i;
  uint16_t tmp[4];
  uint16_t fademin_cmy = 255;

  /* Blend CMYK by addition, then
  * normalise CMY into K */
  for (i = 0; i < 4; i++) {
    tmp[i]  = in1[i] + in2[i];
    if (tmp[i] < fademin_cmy)
    fademin_cmy = tmp[i];
  }
  for (i = 0; i < 3; i++) {
    if (tmp[i] - fademin_cmy > 255)
    out[i] = 255;
    else
    out[i] = tmp[i] - fademin_cmy;
  } 
  out[3] = 0;

}

void average_cmyk (const int in1[], const int in2[], int out[])
{
  int i;
  uint16_t tmp[4];
  uint16_t fademin_cmy = 255;

  /* Blend CMYK by addition, then
  * normalise CMY into K */
  for (i = 0; i < 4; i++) {
    tmp[i]  = in1[i] + in2[i];
    tmp[i]  = tmp[i]/2;
    if (tmp[i] < fademin_cmy)
    fademin_cmy = tmp[i];
  }
  for (i = 0; i < 3; i++) {
    if (tmp[i] - fademin_cmy > 255)
    out[i] = 255;
    else
    out[i] = tmp[i] - fademin_cmy;
  }
  if (tmp[3] + fademin_cmy > 255)
  out[3] = 255;
  else
  out[3] = tmp[3] + fademin_cmy;
}

void average_pix_rgb(const int in1[], const int in2[], int out[]) {

  for(int i=0; i<3; i++){
    out[i] = in1[i] + in2[i];
    out[i] /= 2;

  }
}

// int addpixrgb(int hours[3], int fademins[3]){
//  //extracts and averages each channel of light
//      int newr = (((fademins >> 16) & 0xff)+((hours >> 16) & 0xff))/2;
//     int newg = (((fademins >>  8) & 0xff)+((hours >>  8) & 0xff))/2;
//     int newb = (((fademins      ) & 0xff)+((hours      ) & 0xff))/2; 
// return strip.Color(newr, newg, newb);

// }  



// int addpixcym(int hours, int fademins){

//     int newr = 256-(256-((fademins >> 16) & 0xff)+256-((hours >> 16) & 0xff))/2;
//     int newg =  256-(256-((fademins >>  8) & 0xff)+256-((hours >>  8) & 0xff))/2;
//     int newb =  256-(256-((fademins      ) & 0xff)+256-((hours      ) & 0xff))/2; 
//     return strip.Color(newr, newg, newb);

// }

// void interpolate_pix_array(int rgb1[], int rgb2[], int arraysize, int fadeoutput[][3])
// {

//     //this code will take two RGB colour inputs and output a string of pixels that fades smoothly between them
//     for(int i = 0; i < arraysize; i++){
//     //calculate each colour of each pixel. We must cast as a float to avoid all sorts of weirdness I don't understand about integer maths
//         for(int j=0; j<3; j++){
//             fadeoutput[i][j] = (float)i*((float)rgb2[j]-(float)rgb1[j])/(float)arraysize+(float)rgb1[j];
//             //Serial.println(fadeoutput[i][j]);
//             //fadeoutput[i][j]/=16;
//         }

//     }

// }

// void testinterpolate()
// {

//     int addedpix[3];            

//     int testpix[3];
//     int c1[3] = {255, 255, 0};
//     int c2[3] = {100, 100, 100};


//     int testbase[3];
//     int b1[3] = {0, 255, 255};
//     int b2[3] = {100, 100, 100};
//     //interpolate_pix(c1, c2, 120, testarray);

//     // int sendarray[120][3];
//     for(int hour=0; hour<120; hour++){


//         for(int i=0; i<120; i++){

//             for(int j=0; j<120; j++){

//                 interpolate_pix(c1, c2, 120, (i+j)%120, testpix);
//                 interpolate_pix(b1, b2, 120, (j+hour)%120, testbase);

//                 add_pix_cmyk(testpix, testbase, addedpix);

//                 strip.setPixelColor(j,addedpix[0],addedpix[1],addedpix[2]);
//                 //Serial.println(addedpix[j]);

//             }
//             strip.show();
//             //delay(10);
//         }
//     }
// }
// void rotatearray(int arrayin[][3], int shift, int sizeofarrayin)
// {
//     //this function will rotate an array, typically used to place the "hand" of a clock.

//     //int sizeofarrayin = 120;

//     //Serial.println("size of array is");
//     //Serial.println(sizeofarrayin);
//     int temparray[shift][3];

//     //store the shifted variables in a temp array
//     for(int i=0; i<shift; i++){
//         for(int j=0; j<3; j++){
//             temparray[i][j]=arrayin[i][j];
//         }          
//     }
//     //shift the rest of the array

//     for(int i=0; i<(sizeofarrayin); i++){
//         for(int j=0; j<3; j++){
//             arrayin[i][j]=arrayin[i+shift][j];
//         }
//     }

//     //put back the end of the array
//     for(int i=0; i<shift; i++){
//         for(int j=0; j<3; j++){
//             arrayin[i+sizeofarrayin-shift][j]=temparray[i][j];
//         }
//     }
// }
