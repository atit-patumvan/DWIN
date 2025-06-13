#ifndef MSC_H
#define MSC_H

#include <Wire.h>
#include <Adafruit_AS7341.h>

struct AS7341_Data {
    uint16_t f1_415nm, f2_445nm, f3_480nm, f4_515nm, f5_555nm, f6_590nm, f7_630nm, f8_680nm, clear, nir;
    bool valid;
};

class msc {
public:
    msc(uint8_t tcaAddr = 0x70);
    ~msc();

    bool begin();
    bool initializeSensor(uint8_t channel);
    AS7341_Data readSensor(uint8_t channel, uint16_t atime, uint16_t astep, as7341_gain_t gain);
    void showSensorData(const AS7341_Data& data);

    // JSON wrapper, uuid as first parameter
    String readSensorJSON(const String& uuid, uint8_t channel, uint16_t atime, uint16_t astep, as7341_gain_t gain);

    Adafruit_AS7341& getAS7341();

private:
    void selectChannel(uint8_t channel);
    void setLightSource(uint8_t channel, bool on);
    bool checkSensor(uint8_t channel);

    uint8_t _tcaAddr;
    static const uint8_t _sensorAddr = 0x39;
    static const uint8_t _channels[3];
    static const uint8_t _lightSource[3];
    Adafruit_AS7341 _as7341;
};

#endif // MSC_H
