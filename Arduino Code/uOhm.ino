#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Defining OLED screen size
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 


// Initialising Variables for moving window filtering of current sensor output
#define WINDOW_SIZE 20
int INDEX = 0;
int VALUE = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];
int AVERAGED = 0;

// Initialising Variables for moving window filtering of instrumentation amp output
#define WINDOW_SIZE_V 20
int INDEX_V = 0;
int VALUE_V = 0;
int SUM_V = 0;
int READINGS_V[WINDOW_SIZE_V];
int AVERAGED_V = 0;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);// Initialising OLED display

float ADCin0 = A6;//Voltage sense input pin
float ADCin1 = A7;//Current sense input pin
float DFac = 0 ;//Gain-wise division factor(Initial value)
float ADCValV;//Voltage value
float ADCValI;//Current value
float Iadd = 0.11;//Addition factor for current
float Res = 0;//Calculated resistance between points of the probe (Initial value)
//Initialising pins for gain selector, GainXYZ/valXYZ correspond to a gain resistor value of XYZ Ohms
const float Gain68 = 3;
const float Gain175 = 4;
const float Gain330 = 5;
const float Gain660 = 6;
const float Gain1k = 7;
float val68;
float val175;
float val330;
float val660;
float val1k;



void setup() {
  Serial.begin(9600);//UART Initialization

  // initialize OLED display with address 0x3C for 128x64
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000);         //Display Initialisation 
  display.clearDisplay(); 
  
}

void loop() {
 //Selection of division factor according to gain value selected in gain selector
  val68 = digitalRead(Gain68);
  val175 = digitalRead(Gain175);
  val330 = digitalRead(Gain330);
  val660 = digitalRead(Gain660);
  val1k = digitalRead(Gain1k);
  if (val68 == 1.00) DFac = 171;
  if (val175 == 1.00) DFac = 64;
  if (val330 == 1.00) DFac = 33.66;
  if (val660 == 1.00) DFac = 16;
  if (val1k == 1.00) DFac = 10.37;
  display.clearDisplay(); 
  display.setTextSize(2);          
  display.setTextColor(WHITE);     
  display.setCursor(0,0);       

//Moving window averaging of ACS712 current sensor output
  SUM = SUM - READINGS[INDEX];       // Remove the oldest entry from the sum
  VALUE = analogRead(ADCin1);        // Read the next sensor value
  READINGS[INDEX] = VALUE;           // Add the newest reading to the window
  SUM = SUM + VALUE;                 // Add the newest reading to the sum
  INDEX = (INDEX+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size
  AVERAGED = SUM / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

//Moving window averaging of Instrumentation Amplifier output
  SUM_V = SUM_V - READINGS_V[INDEX_V];       // Remove the oldest entry from the sum
  VALUE_V = analogRead(ADCin0);        // Read the next sensor value
  READINGS_V[INDEX_V] = VALUE_V;           // Add the newest reading to the window
  SUM_V = SUM_V + VALUE_V;                 // Add the newest reading to the sum
  INDEX_V = (INDEX_V+1) % WINDOW_SIZE_V;   // Increment the index, and wrap to 0 if it exceeds the window size
  AVERAGED_V = SUM_V / WINDOW_SIZE_V;      // Divide the sum of the window by the window size for the result

  ADCValV = AVERAGED_V/DFac; //Calculation of voltage from ADC value and gain-specific division factor
  ADCValI = ((((AVERAGED)-512.00)/38.00)+Iadd); //Calculation of current from smoothed ADC value 
  Serial.println(ADCValI);           //UART output of calculated current
  delay(25); 


  Res = ADCValV/ADCValI;              //Resistance calculation
//Displaying derived values on OLED display
  display.print(ADCValV);             
  display.println("  mV");
  display.print(ADCValI);
  display.println("  A");
  display.println("R (mOhm) -");
  display.println(Res);
  display.display();
  
}
