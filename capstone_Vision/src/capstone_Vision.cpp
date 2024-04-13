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


const int PIXELCOUNT = 20;                          // Total number of NeoPixels
const int DELAYLIGHT = 500;
int neopixel_1;

const int TOUCHPIN = D18;                            // Touch Sensor

const int vibrationSensor = D4;                      // Vibration Sensor 

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

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);






// setup() runs once, when the device is first turned on
void setup() {
Serial.begin(9600);
  waitFor(Serial.isConnected,1000);
  BLE.on();

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
  delay(5000);                      // Scan for nearby devices every 5 seconds


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
  delay(1000);



 //for (neopixel_1=0; neopixel_1<=19; neopixel_1++){
  
  //pixel.setPixelColor(neopixel_1,144,66,0);
  //pixel.show ();
  //delay (DELAYLIGHT);
  //pixel.clear();
  //pixel.show();
  //}

}


