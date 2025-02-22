/*
This sketch creates a simple data logger. It will print out data (every 5~6 seconds) to the
printer as long as the printer BLE is connected. If the printer disconnects
the data restart after reconnect (any data while disconnected will be lost).
If the micro-controller restarts, the entire sketch restarts.
*/
#include <AbleTP.h>  // must complile before TFT_sSPI (if used) due to conflicting Adafruit_GFX libraries....

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>

#include "datalog.h"
#include <Fonts\FreeSansBoldOblique12pt7b.h> // imported from Adafruit_GFX.h

// Sketch picks the first of these 3 to scan for that is not empty "".
#define UUID "18f0"                         // "18f0"
#define DEVICE_ADDRESS "86:67:7a:b3:d9:38"  //"86:67:7a:b3:d9:38"
#define DEVICE_NAME "PT-210_D938"

// TPchar UUID must be match your printer service and characteristic
#define SERVUUID "18f0"
#define CHARUUID "2af1"

BLEDevice peripheral;
BLEService TPserv(SERVUUID);
BLECharacteristic TPChar(CHARUUID, BLEWrite, BLEWriteWithoutResponse, "150");
AbleTP tp;
bool validprinter = false;
bool headerPrinted = false;

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
    if (headerPrinted == 0) {
      tp.reset();
      tp.setAlignment(1);  // aligns canvas to center of paper
      delay(100);
      tp.printImage(datalog, 320, 194, false);
      tp.canvasTextDatum(T_L);
      tp.canvasSetSize(360, 35);
      tp.canvasSetFont(&FreeSansBoldOblique12pt7b);
      tp.canvasSetCursor(0, 0);
      tp.canvasPrintText("MILLIS");
      tp.canvasSetCursor(140, 0);
      tp.canvasPrintText("DATA");
      tp.canvasSetCursor(260, 0);
      tp.canvasPrintText("GRAPH");
      tp.canvasFillRect(0, 28, 360, 4, 1);
      tp.printCanvas();
      delay(100);
      headerPrinted = 1;
    }
    tp.setAlignment(1);  // aligns canvas to center of paper
    int canvasH = 30;
    tp.canvasSetSize(360, canvasH);
    tp.canvasTextDatum(T_L);
    tp.canvasSetFont(&FreeSansBoldOblique12pt7b);
    tp.canvasSetCursor(0, 0);
    tp.canvasPrintText(String(millis()).c_str());
    tp.canvasDrawVLine(259, 0, canvasH, 1);
    tp.canvasDrawVLine(309, 0, canvasH, 1);
    tp.canvasDrawVLine(359, 0, canvasH, 1);
    int delayms = random(5000, 6000);
    tp.canvasSetCursor(140, 0);
    tp.canvasPrintText(String(delayms).c_str());
    int barwide = (delayms - 5000) / 10;
    tp.canvasFillRect(259, 5, barwide, canvasH - 10, 1);
    tp.printCanvas();
    Serial.println(String(delayms).c_str());
    delay(delayms);
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
//Connects to printer being scanned for and verifies TPServ and TPChar UUIDs //
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
//  If printer disconnects, automatically starts searching for it again  //
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
