/* 
 * Project Capstone Vision Home Portal 
 * Author: Andres S Cordova
 * Date: 04-APRIL-24
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


int rssi;                               //Strength of Bluetooth
int count;
int rssiArr[4] = {};
int arrayCounter;
int average;
float signalStrength;



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


int inputPin = D4;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status



SYSTEM_MODE(SEMI_AUTOMATIC);      //Using BLE and not Wifi


Button button(BUTTONPIN);                                     //Button 
Adafruit_SSD1306 display(OLED_RESET);                         //OlED Screen
Adafruit_BME280 bme;                                          //BME sensor 
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);           //Neopixel
IoTTimer pixelTimer;

// These UUIDs were defined by Nordic Semiconductor and are now the defacto standard for
// UART-like services over BLE. Many apps support the UUIDs now, like the Adafruit Bluefruit app.
const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

const size_t SCAN_RESULT_MAX = 10;
BleScanResult scanResults[SCAN_RESULT_MAX];
BleAdvertisingData data;

void scanRssi(); 
void rssiAverage();
void rssiRange();
void pixelFill(int start,int end, int color);


//void pixelFill (int start, int end, int hexcolor, int pixelBrightness);
int segment;


void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected,2500);
  Serial.println("Ready to Go\n");
  
  
  BLE.on();
  data.appendServiceUUID(rxUuid);
  BLE.advertise(&data);
  BLE.setTxPower(-20);

  /*WiFi.on();                                    //Wemo & Hue
  WiFi.clearCredentials();
  WiFi.setCredentials("IoTNetwork");

  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
    }
      Serial.printf("\n\n");
*/
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);          //OLED
  display.setRotation(2);
  display.setTextSize(2);
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

  //pixelTimer.startTimer(500);       // Pixel Timer
  pixel.begin();
  pixel.setBrightness(255);
  pixel.show(); 

  pinMode(inputPin, INPUT);     // declare sensor as input


}


void loop() {
  
  scanRssi();                                 // Scan for nearby devices every 5 seconds
  rssiAverage();
  rssiRange();
  

/*if (button.isClicked()){
  buttonOnOff =! buttonOnOff;
  Serial.printf("Click\n");
  }
    if (buttonOnOff){
      wemoWrite(0,HIGH);
      }
      else {
      wemoWrite(0,LOW);
       }*/
        display.setCursor(14,0);
        tempF = (bme.readTemperature()*(9.0/5.0)+32);   //deg C 
        display.printf("HOME\n");
        display.printf("PORTAL\n");
        display.printf("Temp %0.1f\n",tempF);
        display.display();
        display.clearDisplay();

    /*if(pixelTimer.isTimerReady()){
      pixel.clear();
      pixel.setPixelColor(neopixel_1,green);
      pixel.show();
      neopixel_1 ++;

    if (neopixel_1 >= 11){
     neopixel_1 = 0;
    }
  }*/


  /*val = digitalRead(inputPin);  // read input value
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
 }*/
    
}


void scanRssi(){
  count = BLE.scan(scanResults, SCAN_RESULT_MAX);
  Serial.printf("%i Devices Found\n", count);
  if(count > 0){
    for(int ii = 0; ii < count; ii++){
      //Serial.printf("BLE Address: %02X:%02X:%02X:%02X:%02X:%02X --- rssi: %i\n", scanResults[ii].address()[0], scanResults[ii].address()[1], scanResults[ii].address()[2], scanResults[ii].address()[3], scanResults[ii].address()[4], scanResults[ii].address()[5], scanResults[ii].rssi());
      BleUuid foundServiceUuid;
      size_t svcCount = scanResults[ii].advertisingData().serviceUUID(&foundServiceUuid,1);
      if(svcCount > 0 && foundServiceUuid == rxUuid){
        rssi = scanResults[ii].rssi();
        Serial.printf("RSSI: %i\n",rssi); 
      }      
    }
  }
}

void rssiAverage(){
  int sum = 0;
  rssiArr[arrayCounter] = rssi;
  Serial.printf("arr element: %i--- arr value: %i\n", arrayCounter, rssiArr[arrayCounter]);
  for(int i = 0; i <= 3; i++){
    sum+=rssiArr[i];
  }
  Serial.printf("sum %i\n", sum);
  average=sum/4;
  Serial.printf("avg: %i\n",average);
  arrayCounter++;
  if(arrayCounter >= 3){
    arrayCounter=0;
    sum = sum - rssiArr[arrayCounter];
  }
}





void rssiRange() {
   if(average > -30){
    pixelFill (0,11,green);
      pixel.show();

  }
  if((average < -30) && (average >= -45)){
    pixelFill (0,11,magenta);
      pixel.show();
  }
 
 if((average < -45) && (average >= -60)){
    pixelFill (0,11,blue);
      pixel.show();
 }
  
  if(average < -60){
    pixelFill (0,11,red);
      pixel.show();
  }
 }


void pixelFill(int start,int end, int color){
  int neopixel_1;
  pixel.clear();
for (neopixel_1=start; neopixel_1<=end; neopixel_1++){
  pixel.setPixelColor(neopixel_1,color); 
  }
 pixel.show ();
}
