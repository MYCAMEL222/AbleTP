
# AbleTP
A Thermal Printer library using ArduinoBLE and Adafruit GFX to control MPTII style printers.

Testing was done using a GOOJPRT PT-210. It runs the MPTII firmware.

![Untitled design](https://github.com/user-attachments/assets/2e5e4e18-1b1e-4957-b95b-a8c34ccc7023)

ArduinoBLE allows multiple methods of scanning to find the proper printer (MAC, Name and UUID).
If you need to find printer info like a scannable printer name or MAC address, you can run a test print and that info will be printed out (most printers will do this). 


To run a test print:

1.  Make sure printer power is OFF.
2.  Hold down the form feed button.
3.  Press the power button to ON.
4.  Printer should automatically print a test page.

To find printer UUID, you can use a BLE scanner app like…

https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&pcampaignid=web_share

You may be able to use these sketches with other Thermal Printers by changing the UUID settings to match your printer. That has not been tested.




*** The example below creates a simple data logger. It will print out data (every 5~6 seconds) to the
printer as long as the printer BLE is connected. If the printer disconnects
the data restart after reconnect (any data while disconnected will be lost).
If the micro-controller restarts, the entire sketch restarts. ***

![rotdatalogger](https://github.com/user-attachments/assets/e38fea1d-8a5d-430b-9ed4-b25ad4c3372b)


*** The example below draws Adafruit_GFX graphics to the printer.
It uses the controllers memory as a buffer (canvas).
Once the image is drawn to the canvas, the canvas is printed to the printer via BLE. ***

![rotgfx](https://github.com/user-attachments/assets/1d5ef22e-d6eb-48f7-a70e-f1b801717e48)


*** The example below prints out images and custom text ***

![rotimages](https://github.com/user-attachments/assets/beb9a02e-5da0-485f-a2e4-24db253c650e)


*** The example below prints the printer's built in barcode functions. ***

![rotbarcodes](https://github.com/user-attachments/assets/980a7e42-c85f-4965-aba6-6bb9ef0da0f1)


*** The example below prints many of the normal ESC/POS commands to the connected BLE printer. ***

![rotnormal_commands](https://github.com/user-attachments/assets/61a1f6b8-e205-4b61-9ce0-b2e3fa0a18f7)




