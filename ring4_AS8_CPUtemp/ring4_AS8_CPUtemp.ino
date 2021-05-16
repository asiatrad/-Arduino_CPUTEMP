//#define DEBUG
//#include <Adafruit_GFX_AS8.h>    // Core graphics library
#include <Adafruit_ILI9341_AS8.h> // Hardware-specific library

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define ILI9341_LIGHTGREY 0x8450

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

Adafruit_ILI9341_AS8 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

unsigned long runTime = 0;

float sx = 0, sy = 0;
uint16_t x0 = 0, x1 = 0, y0 = 0, y1 = 0;

void setup(){
  tft.reset();
  delay(100);
  tft.begin(0x9341);
  tft.setRotation(3);
  Serial.begin(9600);
}

void loop() {
  if (millis() - runTime >= 1000) { // Execute every 10ms
    runTime = millis();
 
// シリアルポートからデータを受け取ったら
   if (Serial.available() >= 3) {
    if ( Serial.read() == 'H') {   
    int d = Serial.read();
    int f = Serial.read();      
    float cputmp = d  +  (f *  1.0 / 100) ; 
    float PV = cputmp; 
        int xpos = 10, ypos = 5, gap = 34, radius = 100;
        ringMeter(PV,0,110, xpos,ypos,radius,"CPU C",GREEN2RED); // Draw analogue meter
    Serial.println(PV);
    }
   }
// Draw two FAN meters
  if (Serial.available() >= 3) {
    if ( Serial.read() == 'R') {     
    int d = Serial.read();
    int f = Serial.read(); 
    int  fanrpm = d * 256 +  f ;  //reading3 * (3.0 / 1023);
    int  B3 = fanrpm;
        int xpos = 190, ypos = 130, gap = 34, radius = 60;
        xpos = gap + ringMeter(B3, 200, 2200, xpos, ypos, radius, "FAN RPM", BLUE2RED); // Draw analogue meter
    Serial.println(B3);
    }
  }
 }
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(float value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 4;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int text_colour = 0; // To hold the text colour

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 3; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 8; // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = ILI9341_RED; break; // Fixed colour
      case 1: colour = ILI9341_GREEN; break; // Fixed colour
      case 2: colour = ILI9341_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = ILI9341_BLUE; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_GREY);
    }
  }

  // Convert value to a string
  char buf[10];
  if( value < 999){
//  byte len = 4; if (value > 999) len = 5;
  dtostrf(value, 4, 2, buf);
  }
  else{
  dtostrf(value, 5, 0, buf);
  }

  // Set the text colour to default
  tft.setTextColor(ILI9341_WHITE, ILI9341_LIGHTGREY);
  // Uncomment next line to set the text colour to the last segment value!
  tft.setTextColor(text_colour, ILI9341_LIGHTGREY);
  
  // Print value, if the meter is large then use big font 6, othewise use 4
  if (r > 84) tft.drawCentreString(buf, x - 5, y - 20, 6); // Value in middle
  else tft.drawCentreString(buf, x - 5, y - 20, 4); // Value in middle

  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(ILI9341_WHITE, ILI9341_LIGHTGREY);
  if (r > 84) tft.drawCentreString(units, x, y + 30, 4); // Units display
  else tft.drawCentreString(units, x, y + 5, 2); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase) {
  return sin(phase * 0.0174532925);
}
