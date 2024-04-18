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
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"
#include "IoTTimer.h"

int rssi;                                           //Strength of Bluetooth
int count;
int rssiArr[4] = {};
int arrayCounter;
int average;
float signalStrength;

const int BUFSIZE = 50; 

const int PIXELCOUNT = 12;                          // Total number of NeoPixels
const int DELAYLIGHT = 500;


const int TOUCHPIN = D18;                            // Touch Sensor
int lastTime;

const int vibrationSensor = A5;                      // Vibration Sensor 
int val = 0;
int sensorValue;

byte accel_x_h, accel_x_l;                          //variables to store the individual btyes
int16_t accel_x;                                    //variable to store the x-acceleration
byte accel_y_h, accel_y_l;
int16_t accel_y;
byte accel_z_h, accel_z_l;
int16_t accel_z;

float accelGx;
float accelGy;
float accelGz;

float shockValue;                                   // Shock Value 


// Global State 
TCPClient TheClient; 

const int MPU_ADDR = (0x68);
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);

// MQTT Server and Wifi Login
IoTTimer plantTimer;
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

//Feeds 
//Adafruit_MQTT_Subscribe cancelAlert = Adafruit_MQTT_Subscribe(&mqtt,AIO_USERNAME "/feeds/cancel-alert");

Adafruit_MQTT_Publish impactAlert = Adafruit_MQTT_Publish(&mqtt,AIO_USERNAME "/feeds/impact-alert");
Adafruit_MQTT_Publish cancelAlert = Adafruit_MQTT_Publish(&mqtt,AIO_USERNAME "/feeds/cancel-alert");


//Functions
void MQTT_connect();
bool MQTT_ping(); 


const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

void onDataReceived (const uint8_t* data , size_t len , const BlePeerDevice& peer , void *context);
void scanRssi(); 
void rssiAverage();
void rssiRange();
void pixelFill(int start,int end, int color);


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

  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  
  //mqtt.subscribe(&cancelAlert);                          //MQTT subscription
  //mqtt.publish(&impactAlert);
  //mqtt.publish(&cancelAlert);


  BLE.on();
  data.appendServiceUUID(rxUuid);
  BLE.advertise(&data);
  BLE.setTxPower(-20);

  Serial.printf("Photon2 BLE Address: %s\n", BLE.address().toString().c_str());


  pinMode(TOUCHPIN, INPUT);                    // Touch Pin 
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(vibrationSensor, OUTPUT);             // Vibration Sensor
  pinSetDriveStrength(D14, DriveStrength::HIGH);

  Wire.begin();                              //Begin I2C communications 
  Wire.beginTransmission (MPU_ADDR);         //Begin transmission to MPU
  Wire.write (0x6B);
  Wire.write (0x00);
  Wire.endTransmission(true);
  

  pixel.begin();
  pixel.setBrightness(45);
  pixel.show(); 
  
  }



void loop() {
  int state = digitalRead(TOUCHPIN);
  digitalWrite(LED_BUILTIN, state);
  Serial.printf("Touch Sensor %i\n",state);
  if (state == 1){
    if(mqtt.Update()) {
      cancelAlert.publish("I am Ok cancel Alert");
  }
  }
  else {
    cancelAlert.publish(" ");
  }
                                    

  scanRssi();                                 // Scan for nearby devices every 5 seconds
  rssiAverage();
  rssiRange();

  MQTT_connect();                             // wifi and adafruit publish 
  MQTT_ping();

 

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

  shockValue = sqrt ((accelGx*accelGx)+(accelGy*accelGy)+(accelGz*accelGz));
  Serial.printf("Value of shock %f \n",shockValue);
 
  
  if (shockValue > 1.8){
    if(mqtt.Update()) {
      impactAlert.publish(shockValue);
  }
  }
  if((millis()-lastTime > 10000)) {
   if(mqtt.Update()) {
     impactAlert.publish(shockValue);
   
    } 
    lastTime = millis();
  }
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
    analogWrite(vibrationSensor,0);
    pixelFill (0,11,green);
      pixel.show();

  }
  if((average < -30) && (average >= -45)){
    analogWrite(vibrationSensor,255);
    pixelFill (0,11,magenta);
      pixel.show();
  }
 
 if((average < -45) && (average >= -60)){
    analogWrite(vibrationSensor,255);
    pixelFill (0,11,blue);
      pixel.show();
  }
  
  if(average < -60){
    analogWrite(vibrationSensor,0);
    pixelFill (0,11,red);
      pixel.show();

  }
  //pixel.show();
  //hexColorMap = map(hexColorMap, 0x00FF00, 0xFF0000, -40, -70);
 }


void pixelFill(int start,int end, int color){
  int neopixel_1;
  pixel.clear();
for (neopixel_1=start; neopixel_1<=end; neopixel_1++){
  pixel.setPixelColor(neopixel_1,color); 
  }
 pixel.show ();
}


void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
    Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) {                         // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);                                              // wait 5 seconds and try again
  }
    Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}





