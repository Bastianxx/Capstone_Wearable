/* 
 * Project Test Capstone 
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"



SYSTEM_MODE(SEMI_AUTOMATIC);

#define MAX_DEVICES 10 // Maximum number of devices to track
#define ADDRESS_LENGTH 18 // Length of the device address string
#define RSSI_THRESHOLD -80 // Example RSSI threshold for filtering


struct Device {
  char address[ADDRESS_LENGTH];
  int rssi;
};

Device devices[MAX_DEVICES]; // Array to store device information
int deviceCount = 0; // Number of devices currently being tracked
void scanResultCallback(const BleScanResult *scanResults, void *context);

void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 10000);
  BLE.on();
}
void loop() {
  BLE.scan(scanResultCallback);
  delay(5000); // Scan for nearby devices every 5 seconds
}


void scanResultCallback(const BleScanResult *scanResults, void *context) {
  // Store or update the RSSI value for the device address
  for (int i = 0; i < deviceCount; i++) {
    if (strcmp(devices[i].address, scanResults->address.toString().c_str()) == 0) {
      devices[i].rssi = scanResults->rssi;
      return; // Exit the function if address is found and updated
    }
  }
  // Add a new device if it's not already in the list
  if (deviceCount < MAX_DEVICES) {
    strncpy(devices[deviceCount].address, scanResults->address.toString().c_str(), ADDRESS_LENGTH);
    devices[deviceCount].rssi = scanResults->rssi;
    deviceCount++;
  }
  // Print the address and RSSI of all devices
  Serial.println("Devices:");
  for (int i = 0; i < deviceCount; i++) {
    Serial.print("Address: ");
    Serial.print(devices[i].address);
    Serial.print(" | RSSI: ");
    Serial.println(devices[i].rssi);
  }
}