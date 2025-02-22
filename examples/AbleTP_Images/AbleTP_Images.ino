/*
This example prints out images and custom text....
*/
#include <AbleTP.h>  // must complile before TFT_sSPI (if used) due to conflicting Adafruit_GFX libraries....

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>

#include "abletplogo.h"                       // image file
#include "PermanentMarker_Regular20pt7b.h"    // User generated custom text in PermanentMarker_Regular20pt7b.h
#include <Fonts\FreeSansBoldOblique24pt7b.h>  // Imported from the Adafruit_GFX library
#include <Fonts\FreeMonoBold18pt7b.h>         // Imported from the Adafruit_GFX library
#include <Fonts\FreeSerifItalic12pt7b.h>      // Imported from the Adafruit_GFX library

// Sketch picks the first of the 3 below to scan for (that is not empty "").
#define UUID "18f0"                         // "18f0"
#define DEVICE_ADDRESS "86:67:7a:b3:d9:38"  //"86:67:7a:b3:d9:38"
#define DEVICE_NAME ""                      // "PT-210_D938"

// TPchar UUID must be match your printer service and characteristic
#define SERVUUID "18f0"
#define CHARUUID "2af1"

BLEDevice peripheral;
BLEService TPserv(SERVUUID);
BLECharacteristic TPChar(CHARUUID, BLEWrite, BLEWriteWithoutResponse, "150");
AbleTP tp;
bool validprinter = false;

const GFXfont* PM20 = &PermanentMarker_Regular20pt7b;

void setup(void) {

  Serial.begin(115200);
  while (!Serial)
    ;

  BLE.begin();
  BLE.setEventHandler(BLEDiscovered, blePeripheralDiscoverHandler);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  if (strcmp(UUID, "") != 0) {
    BLE.scanForUuid(UUID, true);
    Serial.println(String("BLE.scanForUuid: ") + UUID);
  } else if (strcmp(DEVICE_ADDRESS, "") != 0) {
    BLE.scanForAddress(DEVICE_ADDRESS, true);
    Serial.println(String("BLE.scanForAddress: ") + DEVICE_ADDRESS);
  } else if (strcmp(DEVICE_NAME, "") != 0) {
    BLE.scanForName(DEVICE_NAME, true);
    Serial.println(String("BLE.scanForName: ") + DEVICE_NAME);
  } else {
    Serial.println("No valid scan method defined.");
  }

  BLEDevice peripheral = BLE.available();
  tp.begin(TPChar);  // Pass TPChar to initialize the AbleTP object
}
void loop() {

  // validprinter is determined by blePeripheralConnectHandler....
  if (validprinter) {
    Serial.println("PRINTING..............\n\n");
    tp.reset();

    tp.setAlignment(1);  //centered
    tp.setBold(1);       //bold on
    tp.setLineSpace(50);
    tp.printText("-- Normal text in bold --\n");

    tp.setAlignment(0);  //Left
    tp.printCustomText("CUSTOM TEXT", 0, PM20);
    delay(500); // not needed, just slows printing for user effect
    tp.printCustomText("IS REALLY", 50, &FreeSansBoldOblique24pt7b);
    delay(500);
    tp.printCustomText("JUST A", 100, &FreeMonoBold18pt7b);
    delay(500);
    tp.printCustomText("LITTLE IMAGE", 150, &FreeSerifItalic12pt7b);
    delay(500);
    tp.resetLineSpace();

    tp.setTextScale(1, 2);
    tp.printCustomText("Tall Text\n", 0, &FreeSerifItalic12pt7b);
    delay(500);

    tp.setTextScale(2, 1);
    tp.printCustomText("Wide x2", 0, &FreeMonoBold18pt7b);
    delay(500);

    tp.setTextScale(2, 2);
    tp.printCustomText("Wide", 0, &FreeSansBoldOblique24pt7b);
    tp.printCustomText("& Tall", 0, &FreeSansBoldOblique24pt7b);
    delay(500);

    // prints image array named "abletplogo" from PROGMEM found in "abletplogo.h" file
    tp.setAlignment(1);                                           //centered
    tp.printImage(abletplogo, IMAGE_WIDTH, IMAGE_HEIGHT, false);  // normal colors
    delay(500);
    tp.printImage(abletplogo, IMAGE_WIDTH, IMAGE_HEIGHT, true);  // inverted colors
    tp.printText("\n");                                          // linefeed

    validprinter = false;
    Serial.println("DONE PRINTING...................\n\n\n\n\n");
  }
  BLE.poll();
  peripheral.poll();
  delay(10);
}
////  Used to find printer being scanned for
void blePeripheralDiscoverHandler(BLEDevice peripheral) {
  Serial.println("Discovered BLE event");
  BLE.stopScan();
  Serial.println("Discovered ...");
  if (!peripheral.connected()) {
    if (peripheral.connect()) {
      Serial.println("Connected");
    } else {
      Serial.println("Failed to connect!");
      return;
    }
  }
}
//  Connects to printer being scanned for and verifies TPServ and TPChar UUIDs //
void blePeripheralConnectHandler(BLEDevice peripheral) {
  Serial.println("Connection BLE event");
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
  Serial.println();
  Serial.print("Device name: ");
  Serial.println(peripheral.deviceName());
  Serial.print("Device address: ");
  Serial.println(peripheral.address());
  Serial.println();
  if (peripheral.hasService(SERVUUID)) {
    TPserv = peripheral.service(SERVUUID);
    Serial.println(String("TPserv.peripheral.service = ") + SERVUUID);
    if (TPserv.hasCharacteristic(CHARUUID)) {
      TPChar = TPserv.characteristic(CHARUUID);
      Serial.println(String("TPChar.service.characteristic = ") + CHARUUID);
      // bool flag to confirm a valid printer is found and connected///
      validprinter = true;
      Serial.println("Valid printer found!");

    } else {
      validprinter = false;
      peripheral.disconnect();
      peripheral = BLE.available();
      if (strcmp(UUID, "") != 0) {
        BLE.scanForUuid(UUID, true);
      } else if (strcmp(DEVICE_ADDRESS, "") != 0) {
        BLE.scanForAddress(DEVICE_ADDRESS, true);
      } else if (strcmp(DEVICE_NAME, "") != 0) {
        BLE.scanForName(DEVICE_NAME, true);
      } else {
        Serial.println("No valid scan method defined.");
      }
      Serial.println("Valid printer not found :^(");
    }
  }
}
//  If printer disconnects, automatically starts searching for it again by UUID //
void blePeripheralDisconnectHandler(BLEDevice peripheral) {
  Serial.println("Disconnected BLE event");
  validprinter = false;
  peripheral = BLE.available();
  if (strcmp(UUID, "") != 0) {
    BLE.scanForUuid(UUID, true);
  } else if (strcmp(DEVICE_ADDRESS, "") != 0) {
    BLE.scanForAddress(DEVICE_ADDRESS, true);
  } else if (strcmp(DEVICE_NAME, "") != 0) {
    BLE.scanForName(DEVICE_NAME, true);
  } else {
    Serial.println("No valid scan method defined.");
  }
}
