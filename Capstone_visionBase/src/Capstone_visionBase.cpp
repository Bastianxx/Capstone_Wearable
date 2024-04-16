/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "IoTClassroom_CNM.h"
#include "Colors.h"
#include "button.h"
#include "neopixel.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"



const int MYWEMO=0;                     //Wemo and Hue 
const int BUTTONPIN = D14; 
const int BULB=1; 
int color;

bool buttonState;                       //Button 
bool buttonOnOff;

int position;                           
int lastPosition;
int newPosition;


bool status;                            //for OLED & BME 
const int OLED_RESET = -1;
float tempF;
float tempC;

const int PIXELCOUNT = 12;              //for LED Pixels
int neopixel_1;
const int DELAYLIGHT = 500;

int inputPin = D4;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status


// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(MANUAL);

Button button(BUTTONPIN);
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BME280 bme;
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
IoTTimer pixelTimer;




void pixelFill (int start, int end, int hexcolor, int pixelBrightness);
int segment;

// setup() runs once, when the device is first turned on
void setup() {
 Serial.begin(9600);
  waitFor(Serial.isConnected,10000);

  WiFi.on();                                    //Wemo & Hue
  WiFi.clearCredentials();
  WiFi.setCredentials("IoTNetwork");
  
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Serial.printf("\n\n");


  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);          //OLED
  display.setRotation(2);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.display();

  status = bme.begin(0x76);                             // BME
  if (status == false){
  Serial.printf ("BME280 at address 0x%02xfailed to start", 0x76);
  display.printf ("BME280 at address 0x%02xfailed to start", 0x76);
  display.display();
  }
  
  pixelTimer.startTimer(500);       // Pixel Timer
  pixel.begin();
  pixel.setBrightness(255);
  pixel.show(); 

  pinMode(inputPin, INPUT);     // declare sensor as input



}

// loop() runs over and over again, as quickly as it can execute.
void loop() {


   if (button.isClicked()){
      buttonOnOff =! buttonOnOff;
      Serial.printf("Click\n");
    }

    if (buttonOnOff){
      wemoWrite(0,HIGH);

    }

    else {
      wemoWrite(0,LOW);

    }

    display.setCursor(0,0);
  tempF = (bme.readTemperature()*(9.0/5.0)+32);   //deg C 


  display.printf("%0.1f\n",tempF);
  display.display();
  display.clearDisplay();
  

  if(pixelTimer.isTimerReady()){
    pixel.clear();
    pixel.setPixelColor(neopixel_1,56,44,23);
    pixel.show();
    neopixel_1 ++;

  if (neopixel_1 >= 11){
    neopixel_1 = 0;

  }


//for (neopixel_1=0; neopixel_1<=12; neopixel_1++){
  
  //pixel.setPixelColor(neopixel_1,255,66,0);
  //pixel.show ();
  //delay (DELAYLIGHT);
  //pixel.clear();
  //pixel.show();

}

val = digitalRead(inputPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    setHue(BULB, true, HueRed,(255),255);  // turn Hue ON
    if (pirState == LOW) {
      Serial.println("Motion detected!");
      pirState = HIGH;
    }
  } else {
    setHue(BULB, false, HueRed,(255),255); // turn Hue OFF
    if (pirState == HIGH){
      Serial.println("Motion ended!");
      pirState = LOW;
    }
  }
    

}







