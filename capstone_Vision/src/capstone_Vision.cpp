/* 
 * Project Capstone_Vision
 * Author: Andres Sebastian Cordova
 * Date: 08-APRIL-2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Neopixel.h"
#include "Wire.h"
#include "Time.h"
#include "Colors.h"

int rssi;
int count;

const int BUFSIZE = 50; 

const int PIXELCOUNT = 20;                          // Total number of NeoPixels
const int DELAYLIGHT = 500;
int neopixel_1;

const int TOUCHPIN = D18;                            // Touch Sensor

const int vibrationSensor = A5;                      // Vibration Sensor 

byte accel_x_h, accel_x_l;                          //variables to store the individual btyes
int16_t accel_x;                                    //variable to store the x-acceleration
byte accel_y_h, accel_y_l;
int16_t accel_y;
byte accel_z_h, accel_z_l;
int16_t accel_z;

float accelGx;
float accelGy;
float accelGz;

const int MPU_ADDR = (0x68);
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);



const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

void onDataReceived (const uint8_t* data , size_t len , const BlePeerDevice& peer , void *context);
void scanRssi(); 

//BleCharacteristic txCharacteristic("tx", BleCharacteristicProperty::NOTIFY, txUuid, serviceUuid);
//BleCharacteristic rxCharacteristic("rx", BleCharacteristicProperty::WRITE_WO_RSP, rxUuid, serviceUuid, onDataReceived, NULL);


const size_t SCAN_RESULT_MAX = 10;
BleScanResult scanResults[SCAN_RESULT_MAX];
BleAdvertisingData data;



// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);






// setup() runs once, when the device is first turned on
void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected,2500);
  Serial.println("Ready to Go\n");
  
  BLE.on();
  data.appendServiceUUID(rxUuid);
  BLE.advertise(&data);
  BLE.setTxPower(-20);

  Serial.printf("Photon2 BLE Address: %s\n", BLE.address().toString().c_str());


  pinMode(TOUCHPIN, INPUT);                    // Touch Pin 
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(vibrationSensor, INPUT);             // Vibration Sensor

  Wire.begin();                              //Begin I2C communications 
  Wire.beginTransmission (MPU_ADDR);         //Begin transmission to MPU
  Wire.write (0x6B);
  Wire.write (0x00);
  Wire.endTransmission(true);
  




  //pixel.begin();
  //pixel.setBrightness(45);
  //pixel.show(); 
  //neopixel_1 = magenta;




  
  }

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  int state = digitalRead(TOUCHPIN);
  digitalWrite(LED_BUILTIN, state);
  Serial.printf("Touch Sensor %i\n",state);
                                    // Scan for nearby devices every 5 seconds

  scanRssi();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR , 6, true);
  accel_x_h = Wire.read();
  accel_x_l = Wire.read();
  accel_y_h = Wire.read();
  accel_y_l = Wire.read();
  accel_z_h = Wire.read();
  accel_z_l = Wire.read();


  accel_x = accel_x_h << 8 | accel_x_l;
  accel_y = accel_y_h << 8 | accel_y_l;
  accel_z = accel_z_h << 8 | accel_z_l;

  accelGx = (-3.0/-49127)* accel_x;
  accelGy = (-3.0/-49127)* accel_y;
  accelGz = (-3.0/-49127)* accel_z;


  Serial.printf("X-axis acceleration is %f \n",accelGx);
  Serial.printf("Y-axis acceleration is %f \n",accelGy);
  Serial.printf("Z-axis acceleration is %f \n",accelGz);
 



 //for (neopixel_1=0; neopixel_1<=19; neopixel_1++){
  
  //pixel.setPixelColor(neopixel_1,144,66,0);
  //pixel.show ();
  //delay (DELAYLIGHT);
  //pixel.clear();
  //pixel.show();
  //}

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