/* 
 * Project Test Capstone 
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"
#include "neopixel.h"


SYSTEM_MODE(SEMI_AUTOMATIC);

const int PIXEL_PIN = D6;
const int PIXEL_COUNT = 1;

Adafruit_NeoPixel pixel(PIXEL_COUNT, SPI1, WS2812B);

const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
const size_t SCAN_RESULT_MAX = 10;
BleScanResult scanResults[SCAN_RESULT_MAX];
BleAdvertisingData data;

int rssi;
int rssiArr[4] = {};
int arrayCounter;
int count;
int average;


void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected,2500);
  Serial.println("Ready to Go\n");
  BLE.on();
  data.appendServiceUUID(rxUuid);
  BLE.advertise(&data);
  pixel.begin();
  pixel.setBrightness(225);
  pixel.show();
}

void loop() {
  scanRssi();
  colorRange();
  averageRssi();
  //delay(1000);

}

void scanRssi(){
  count = BLE.scan(scanResults, SCAN_RESULT_MAX);
  //Serial.printf("%i Devices Found\n", count);
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

void averageRssi(){
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
  if(arrayCounter >= 4){
    arrayCounter=0;
    sum = sum - rssiArr[arrayCounter];
  }
}

void colorRange(){
  //hexColorMap = map(hexColorMap, 0x00FF00, 0xFF0000, -40, -70);
   if(average > -40){
    pixel.setPixelColor(0, 0,255,0);
      pixel.show();

  }
  if((average < -40) && (average >= -50)){
    pixel.setPixelColor(0, 0,0,255);
      pixel.show();

  }
  if((average < -50) && (average >= -60)){
    pixel.setPixelColor(0, 175,175,0);
      pixel.show();

  }
  if(average < -60){
    pixel.setPixelColor(0, 255,0,0);
      pixel.show();

  }
  //pixel.show();
  //hexColorMap = map(hexColorMap, 0x00FF00, 0xFF0000, -40, -70);

}

















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
