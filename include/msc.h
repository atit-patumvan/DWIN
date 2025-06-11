#ifndef MSC_H
#define MSC_H

#include <Wire.h>

class msc {
public:
    msc(uint8_t tcaAddr = 0x70);
    ~msc();

    bool begin();               // Initialize & check all AS7341, set default config
    bool checkAllSensors();     // Check if all AS7341 exist
    void setDefaultConfig();    // Apply default config to all AS7341
    void readAllSensors();      // Placeholder: read sensor data

private:
    uint8_t _tcaAddr;
    static const uint8_t _sensorAddr = 0x39;
    static const uint8_t _channels[3];

    bool selectChannel(uint8_t channel);
    bool checkSensor();         // Check single AS7341 presence
    void setSensorDefault();    // Set default config for single AS7341
};

#endif // MSC_H
