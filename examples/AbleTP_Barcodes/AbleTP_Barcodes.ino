/*
This example prints the printer's built in barcode functions.
*/
#include <AbleTP.h>
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>

// Sketch picks the first of the 3 below to scan for that is not empty "".
#define UUID ""                             // "18f0"
#define DEVICE_ADDRESS "86:67:7a:b3:d9:38"  //"86:67:7a:b3:d9:38"
#define DEVICE_NAME "PT-210_D938"

// UUIDs below must be match your printer service and characteristic
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
    tp.reset();
    tp.setAlignment(1);  // centered
    tp.setBold(1);
    Serial.println("PRINTING..............\n\n");
    delay(2000);

    tp.printQRCode("https://youtube.com", 1, 4, 1);
    tp.printText("https://youtube.com");
    tp.printText("--- QR Code ---");
    tp.printText("\n");
    Serial.println("--- QR Code ---");
    delay(2000);

    //data should be 11 to 12 numbers long. (0 to 9, numbers only)
    //The 12th checksum digit will be added or changed at the end by the printer.
    tp.printUPCA("66226789012", 2, 80, 0, 2);
    tp.printText("--- UPC A ---\n\n");
    Serial.println("--- UPC A ---");
    delay(2000);

    // data should be 12 OR 13 numbers long. (0 to 9, numbers only)
    // The 13th checksum digit will be added or changed at the end by the printer.
    tp.printEAN13("662266789012", 2, 80, 0, 2);
    tp.printText("--- EAN 13 ---\n\n");
    Serial.println("--- EAN 13 ---");
    delay(2000);

    // Includes all numeric values (0-9),
    // uppercase and lowercase alphabetic characters (A-Z, a-z)
    // punctuation marks.
    tp.printCODE128("aBCD65412!!", 2, 80, 0, 2);
    tp.printText("--- CODE 128 ---\n\n");
    Serial.println("--- CODE 128 ---");
    delay(2000);

    /* All numeric digits (0-9), All uppercase letters (A-Z), %, +, $, /, ., -, space
     Some printers may print a checksum digit in barcode, not in HRI  */
    tp.printCODE39("12B%+/.-CD", 2, 80, 0, 2);
    tp.printText("--- CODE 39 ---\n\n");
    Serial.println("--- CODE 39 ---");
    delay(2000);

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
