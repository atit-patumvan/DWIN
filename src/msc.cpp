#include <Arduino.h>
#include "msc.h"

const uint8_t msc::_channels[3] = {0, 1, 2}; // TCA9548A channels

msc::msc(uint8_t tcaAddr) : _tcaAddr(tcaAddr) {}

msc::~msc() {}

bool msc::selectChannel(uint8_t channel) {
    if (channel > 7) return false;
    Wire.beginTransmission(_tcaAddr);
    Wire.write(1 << channel);
    return (Wire.endTransmission() == 0);
}

bool msc::checkSensor() {
    Wire.beginTransmission(_sensorAddr);
    return (Wire.endTransmission() == 0);
}

bool msc::begin() {
    Wire.begin();
    bool allFound = true;
    for (int i = 0; i < 3; ++i) {
        if (!selectChannel(_channels[i])) {
            allFound = false;
            continue;
        }
        if (!checkSensor()) {
            allFound = false;
        }
    }
    if (allFound) {
        setDefaultConfig();
    }
    return allFound;
}

bool msc::checkAllSensors() {
    bool allPresent = true;
    for (int i = 0; i < 3; ++i) {
        if (!selectChannel(_channels[i]) || !checkSensor()) {
            allPresent = false;
        }
    }
    return allPresent;
}

void msc::setSensorDefault() {
    // Example: Reset command (soft reset), enable, etc.
    // Actual config may depend on AS7341 datasheet/your requirements

    // Enable AS7341 (Power ON)
    Wire.beginTransmission(_sensorAddr);
    Wire.write(0x80);    // ENABLE register
    Wire.write(0x01);    // PON bit
    Wire.endTransmission();

    delay(5);

    // Enable Spectral Measurement (Set SMUX/other defaults)
    Wire.beginTransmission(_sensorAddr);
    Wire.write(0x80);
    Wire.write(0x03); // Power ON + Enable Spectral
    Wire.endTransmission();

    // Set default integration time, gain, etc. as needed here
    // This is a minimal example.
}

void msc::setDefaultConfig() {
    for (int i = 0; i < 3; ++i) {
        if (selectChannel(_channels[i]) && checkSensor()) {
            setSensorDefault();
        }
    }
}

void msc::readAllSensors() {
    // Placeholder: implement sensor reading here if needed
    for (int i = 0; i < 3; ++i) {
        if (selectChannel(_channels[i]) && checkSensor()) {
            // Example: read chip ID register (0x92) for debugging
            Wire.beginTransmission(_sensorAddr);
            Wire.write(0x92);
            if (Wire.endTransmission(false) == 0) { // repeated start
                Wire.requestFrom(_sensorAddr, (uint8_t)1);
                if (Wire.available()) {
                    uint8_t id = Wire.read();
                    Serial.print("AS7341 on channel ");
                    Serial.print(_channels[i]);
                    Serial.print(" ID: 0x");
                    Serial.println(id, HEX);
                }
            }
        }
    }
}
