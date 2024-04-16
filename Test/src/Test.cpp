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