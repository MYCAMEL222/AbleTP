/*
This example draws Adafruit_GFX graphics to the printer.
It uses the controllers memory as a buffer (canvas).
Once the image is drawn to the canvas, the canvas is printed to the printer via BLE.
*/
#include <AbleTP.h>  // must complile before TFT_sSPI (if used) due to conflicting Adafruit_GFX libraries....

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>

#include "IMAGE.h"
#include <Fonts\FreeSansBoldOblique12pt7b.h> // imported from Adafruit_GFX.h

// Sketch picks the first of these 3 to scan for that is not empty "".
#define UUID ""                             // "18f0"
#define DEVICE_ADDRESS "86:67:7a:b3:d9:38"  //"86:67:7a:b3:d9:38"
#define DEVICE_NAME "PT-210_D938"

// UUIDs must be match your printer service and characteristic
#define SERVUUID "18f0"
#define CHARUUID "2af1"

BLEDevice peripheral;
BLEService TPserv(SERVUUID);
BLECharacteristic TPChar(CHARUUID, BLEWrite, BLEWriteWithoutResponse, "150");
AbleTP tp;
bool validprinter = false;

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
    Serial.println("Preparing Canvas\n\n");
    tp.reset();

    int canvaswidth = 360;  // must be div by 8
    int canvasheight = 710;
    int borderwide = 10;
    int texthorz = canvaswidth * .6;

    tp.setAlignment(1);  // aligns canvas to center of paper

    tp.canvasSetSize(canvaswidth, canvasheight);  // Create a 320 wide x 710 tall 1-bit canvas
    tp.canvasFillRect(0, 0, canvaswidth, canvasheight, 1);
    tp.canvasFillRoundRect(borderwide, borderwide, canvaswidth - borderwide * 2, canvasheight - borderwide * 2, borderwide, 0);
    Serial.println("Border Drawn to canvas");

    tp.canvasSetFont(&FreeSansBoldOblique12pt7b);
    tp.canvasTextDatum(T_R);

    tp.canvasSetCursor(texthorz, 40);
    tp.canvasPrintText("DrawRect:");
    tp.canvasDrawRect(texthorz + 10, 30, canvaswidth / 3, 40, 1);
    Serial.println("tp.canvasDrawRect Drawn");

    tp.canvasSetCursor(texthorz, 40 + 50);
    tp.canvasPrintText("FillRect:");
    tp.canvasFillRect(texthorz + 10, 80, canvaswidth / 3, 40, 1);
    Serial.println("tp.canvasFillRect Drawn");

    tp.canvasSetCursor(texthorz, 90 + 50);
    tp.canvasPrintText("DrawRoundRect:");
    tp.canvasDrawRoundRect(texthorz + 10, 80 + 50, canvaswidth / 3, 40, 10, 1);
    Serial.println("tp.canvasDrawRoundRect Drawn");

    tp.canvasSetCursor(texthorz, 90 + 100);
    tp.canvasPrintText("FillRoundRect:");
    tp.canvasFillRoundRect(texthorz + 10, 80 + 100, canvaswidth / 3, 40, 10, 1);
    Serial.println("tp.canvasFillRoundRect Drawn");

    tp.canvasSetCursor(texthorz, 90 + 150);
    tp.canvasPrintText("DrawCircle:");
    tp.canvasDrawCircle(texthorz + 30, 80 + 150 + 20, 20, 1);  // x, y, r, color
    Serial.println("tp.canvasDrawCircle Drawn");

    tp.canvasSetCursor(texthorz, 90 + 200);
    tp.canvasPrintText("FillCircle:");
    tp.canvasFillCircle(texthorz + 30, 80 + 200 + 20, 20, 1);  // x, y, r, color
    Serial.println("tp.canvasFillCircle Drawn");

    tp.canvasSetCursor(texthorz, 90 + 250);
    tp.canvasPrintText("DrawTriangle:");
    tp.canvasDrawTriangle(texthorz + 10, 370, texthorz + 50, 370, texthorz + 30, 330, 1);
    Serial.println("tp.canvasDrawTriangle Drawn");

    tp.canvasSetCursor(texthorz, 90 + 300);
    tp.canvasPrintText("FillTriangle:");
    tp.canvasFillTriangle(texthorz + 10, 420, texthorz + 50, 420, texthorz + 30, 380, 1);
    Serial.println("tp.canvasFillTriangle Drawn");

    tp.canvasSetCursor(texthorz, 90 + 350);
    tp.canvasPrintText("DrawLine:");
    tp.canvasDrawLine(texthorz + 10, 470, (texthorz + 10) + canvaswidth / 3, 470 - 40, 1);
    Serial.println("tp.canvasDrawLine Drawn");

    tp.canvasSetCursor(texthorz, 90 + 400);
    tp.canvasPrintText("DrawHLine:");
    tp.canvasDrawHLine(texthorz + 10, 515, canvaswidth / 3, 1);
    Serial.println("tp.canvasDrawHLine Drawn");

    tp.canvasSetCursor(texthorz, 90 + 450);
    tp.canvasPrintText("DrawVLine:");
    tp.canvasDrawVLine(texthorz + 10, 525, 40, 1);
    Serial.println("tp.canvasDrawVLine Drawn");

    tp.canvasSetCursor(texthorz, 90 + 500);
    tp.canvasPrintText("DrawPixel:");
    Serial.println("Drawing 50 random pixles...Please wait");
    for (int i = 0; i < 50; i++) {
      int rh = random(texthorz + 10, texthorz + 70);
      tp.canvasDrawPixel(rh, 600, 1);
    }
    Serial.println("50 tp.canvasDrawPixel Drawn");

    tp.canvasFillRoundRect(20, 640, canvaswidth - 40, 40, 10, 1);
    tp.canvasFillCircle(50, 660, 15, 0);                
    tp.canvasFillCircle(canvaswidth - 50, 660, 15, 0);  
    tp.canvasTextDatum(M_C);
    tp.canvasSetTextColor(0);
    tp.canvasSetCursor(canvaswidth / 2, 660);
    tp.canvasPrintText("THANK YOU!");

    Serial.println("THANK YOU!");
    Serial.println("Printing Canvas now!");
    tp.printCanvas(); 

    tp.canvasSetSize(240, 360);                             // Create a new 240 wide x 260 tall 1-bit canvas
    tp.canvasPrintImage(testImage, 240, 240, 0, 60, false);  // "testImage" is name of byte array stored in IMAGE.h
    tp.printCanvas();

    // stops printer so it doesn't keep printing
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
