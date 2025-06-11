#include <Arduino.h>
#include "dwin.h"

// Instantiate Dwin with default RX/TX pins (16, 17)
Dwin dwin;

void setup() {
    // Initialize serial for debug
    Serial.begin(115200);
    //while (!Serial) { /* wait for Serial */ }

    // Initialize the DWIN display interface
    delay(2000);
    dwin.init();

    Serial.println("DWIN interface initialized.");
    dwin.pageSwitch(0x01);
    dwin.setBuzzer(false);
}

void loop() {
    // Listen for an event from the display
    DwinEvent evt = dwin.listen();
    if (evt.valid) {
        // Merge high/low bytes into a 16-bit address
        uint16_t addr = Dwin::mergeAddr(evt.startAddr, evt.endAddr);
        uint8_t value = evt.dataVal;
        if(addr!=0x0000){

        Serial.print("Received event at VP ");
        Serial.print(dwin.toHex16(addr));
        Serial.print(" with value ");
        Serial.println(value, HEX);
 
        // Example: Read a text field at that address (length = 16 bytes)
        if(addr==0x5000&&value<0x10){
          dwin.pageSwitch(value);
        }
        // recive Sample Id value is change 
        if(addr==0x0FFF){
          String text = dwin.getTextData(0x1000, 10,2000);
          if (text.length() > 0) {
            Serial.print(" Text: ");
            Serial.println(text);
          }
        }
      }
        
    }
   
    // Small delay to avoid hogging the CPU
    delay(10);
}
