#include <Arduino.h>
#include <Wire.h>

#define TCA_ADDR 0x70  // TCA9548A default I2C address

void tcaSelect(uint8_t channel) {
    if (channel > 7) return;
    Wire.beginTransmission(TCA_ADDR);
    Wire.write(1 << channel);
    Wire.endTransmission();
}

void scanChannel(uint8_t channel) {
    tcaSelect(channel);
    Serial.print("Scanning TCA9548A channel ");
    Serial.print(channel);
    Serial.println(":");
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            Serial.print("  I2C device found at 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.println();
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(26,OUTPUT);
    pinMode(25,OUTPUT);
    pinMode(33,OUTPUT);
    while (!Serial); // Wait for Serial to initialize (for native USB boards)
    Wire.begin();
    digitalWrite(26,LOW);
    digitalWrite(25,LOW);
    digitalWrite(33,LOW);
    Serial.println("\nI2C Scanner with TCA9548A (channels 0-2)");
    for (uint8_t ch = 0; ch <= 2; ch++) {
        scanChannel(ch);
    }
    Serial.println("Scan complete.");
}

void loop() {
    // Nothing needed here
}
