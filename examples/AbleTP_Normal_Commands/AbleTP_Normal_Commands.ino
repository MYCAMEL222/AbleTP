/* 
This example prints many of the normal ESC/POS commands
to the connected BLE printer.
*/
#include <AbleTP.h>
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>

// Sketch picks the first of 3 below to scan for that is not empty "".
#define UUID "18f0"        // "18f0"
#define DEVICE_ADDRESS ""  //"86:67:7a:b3:d9:38"
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

    Serial.println("PRINTING..............\n\n");

    tp.reset();
    tp.setAlignment(0);
    tp.printText("TEXT A, LEFT JUSTIFY!!\n");
    delay(1000);

    tp.setAlignment(2);
    tp.setBold(1);
    tp.printText("BOLD, RIGHT JUSTIFY!!\n");
    delay(1000);

    tp.setAlignment(1);
    tp.setFont(1);
    tp.printText("FONT B, CENTER JUSTIFY!!\n");
    delay(1000);

    tp.setFont(0);
    tp.setUnderline(1);
    tp.printText("UNDERLINE!!\n");
    delay(1000);

    tp.setUnderline(2);
    tp.printText("2x UNDERLINE!!\n");
    delay(1000);

    tp.setInvertColor(1);
    tp.printText("setInvertColor!!\n");
    delay(1000);

    tp.setInvertColor(0);
    tp.setUnderline(0);

    tp.setHorizPos(75);
    tp.printText("TEXT @ 75 PIXELS!!\n");
    delay(1000);

    tp.setAlignment(0);
    tp.setTextScale(1, 2);
    tp.printText("TALL x2\n");
    delay(1000);

    tp.setTextScale(2, 1);
    tp.printText("WIDE x2\n");
    delay(1000);

    tp.setTextScale(2, 2);
    tp.printText("TALL&WIDE x2\n");
    delay(1000);

    tp.setTextScale(6, 2);
    tp.setFont(1);
    tp.printText("FONTB x6\n");
    delay(1000);

    tp.reset();
    tp.printText("BACK TO DEFAULT WITH");
    delay(1000);

    tp.printText("NORMAL LINE SPACE");
    delay(1000);

    tp.setLineSpace(100);
    tp.printText("100 LINE SPACE");
    delay(1000);

    tp.resetLineSpace();
    tp.printText("BACK TO DEFAULT");
    delay(1000);

    tp.setFont(1);
    tp.setAlignment(1);
    tp.setTextScale(2, 2);
    tp.setInvertColor(1);
    tp.printText("\nAbleTP");
    tp.printText("ALL DONE\n\n\n");    
    
    Serial.println("DONE PRINTING...................\n\n\n\n\n");
    validprinter = false;
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
