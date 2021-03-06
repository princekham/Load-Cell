// This is the main Arduino file for load cell to measure the liquid filled by the liquid filler
// The amount of liquid to be filled can be set by the user using encoder button 
// HX711 amplifier is used
// the codes are referred to from https://www.brainy-bits.com/load-cell-and-hx711-with-arduino/

#include "U8glib.h"   // Library for Oled display https://github.com/olikraus/u8glib/
#include "HX711.h"    // Library for Load Cell amplifier board

// Rotary encoder is used to set the gram to weight
volatile boolean TurnDetected;  // variable used to detect rotation of Rotary encoder
volatile int Rotary_Flag=0;     // flag to indicate rotation as occured

// Rotary Encoder Module connections
#define RotaryCLK 2   // Rotary encoder CLK pin connected to pin 2 of Arduino
#define RotaryDT 3    // Rotary encoder DT pin connected to pin 3
#define RotarySW 4    // Rotary encoder Switch pin connected to pin 4

// Button timing variables (added by SKZY for single and double clicks)
int debounce = 50;          // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 500;            // max ms between clicks for a double click event

// Button variables (added by SKZY - only added some parts from the reference page)
boolean buttonVal = HIGH;   // value read from button
boolean buttonLast = HIGH;  // buffered value of the button's previous state
boolean DCwaiting = false;  // whether we're waiting for a double click (down)
boolean DConUp = false;     // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true;    // whether it's OK to do a single click

// HX711 Module connections
#define CLK 5   // CLK of HX711 connected to pin 5 of Arduino
#define DOUT 6  // DOUT of HX711 connected to pin 6 of Arduino

HX711 scale(DOUT, CLK);  // Init of the HX711
U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_FAST);	// Init of the OLED

// Interrupt routine runs if Rotation detected from Rotary encoder
void rotarydetect ()  {
Rotary_Flag=1; // Set Rotary flag from 0 to 1
delay(500);
}

// Run at Startup and when Resetting with Rotary encoder switch
void startupscreen(void) {
  u8g.setFont(u8g_font_unifont);
  u8g.firstPage(); 
  do {
    u8g.drawStr( 0, 10, "Clear Scale");
    u8g.drawStr( 0, 28, "Click to zero...");
  } while( u8g.nextPage() );
}

// Reset Scale to zero
void tare_scale(void) {
  scale.set_scale(-1073000);  //Calibration Factor obtained from calibration sketch
  scale.tare();             //Reset the scale to 0  
}

// Start Displaying information on OLED
void start_scale(void) {
  // The former parts are omitted by SKZY
        u8g.drawStr( 0, 10, temp_current_units);  // Display the current measurement unit
        u8g.setPrintPos(38, 28);
        u8g.print(scale.get_units(3)*unit_conversion, decimal_place);  // Display the average of 3 scale value reading
       while( u8g.nextPage() );
}

void setup(void) {
  
  // Set pinmode for Rotary encoder pins
  pinMode(RotarySW,INPUT_PULLUP);
  pinMode(RotaryCLK,INPUT_PULLUP);
  pinMode(RotaryDT,INPUT_PULLUP);  

  // Attach interrupt 0 (Pin 2 on UNO) to the Rotary Encoder
  attachInterrupt (0,rotarydetect,RISING);   // interrupt 0 always connected to pin 2 on Arduino UNO

  // Rotate screen 180 degrees on OLED, if required
  u8g.setRot180();

  // Set color of OLED to Monochrome
  u8g.setColorIndex(1);

  // Select font to use
  u8g.setFont(u8g_font_unifont);

  String start_count_string="Starting up....";  // Message to display at Startup
  char start_count[15];  // Used to String to Char conversion
  
  // Loop to display counting dots
  for (int x=12; x < 16; x++) {  // Select the first 12 to 16 character of String
    start_count_string.toCharArray(start_count, x);
    u8g.firstPage(); 
    do {
      u8g.drawStr( 0, 10, "ARDUINO SCALE");
      u8g.drawStr( 0, 28, start_count);
    } while( u8g.nextPage() );
    delay(500);  // Delay between dots
  }
}

void loop() {
   // Get button event and act accordingly
   int b = checkButton();
   if (b == 1) Serial.println("single click");
   if (b == 2) Serial.println("double click");
}

int checkButton() {    
   int event = 0;
   buttonVal = digitalRead(buttonPin);
   // Button pressed down
   if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce)
   {
       downTime = millis();
       ignoreUp = false;
       waitForUp = false;
       singleOK = true;
       holdEventPast = false;
       longHoldEventPast = false;
       if ((millis()-upTime) < DCgap && DConUp == false && DCwaiting == true)  DConUp = true;
       else  DConUp = false;
       DCwaiting = false;
   }
   // Button released
   else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce)
   {        
       if (not ignoreUp)
       {
           upTime = millis();
           if (DConUp == false) DCwaiting = true;
           else
           {
               event = 2;
               DConUp = false;
               DCwaiting = false;
               singleOK = false;
           }
       }
   }
   // Test for normal click event: DCgap expired
   if ( buttonVal == HIGH && (millis()-upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true && event != 2)
   {
       event = 1;
       DCwaiting = false;
   }
   // Test for hold
 /*  if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
       // Trigger "normal" hold
       if (not holdEventPast)
       {
           event = 3;
           waitForUp = true;
           ignoreUp = true;
           DConUp = false;
           DCwaiting = false;
           //downTime = millis();
           holdEventPast = true;
       }
       // Trigger "long" hold
       if ((millis() - downTime) >= longHoldTime)
       {
           if (not longHoldEventPast)
           {
               event = 4;
               longHoldEventPast = true;
           }
       }
   }*/
   buttonLast = buttonVal;
   return event;
}
