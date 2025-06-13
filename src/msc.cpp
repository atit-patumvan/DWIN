#include <Arduino.h>
#include "msc.h"

const uint8_t msc::_channels[3] = {0, 1, 2};
const uint8_t msc::_lightSource[3] = {26, 25, 33};

msc::msc(uint8_t tcaAddr) : _tcaAddr(tcaAddr) {}

msc::~msc() {}

void msc::selectChannel(uint8_t channel) {
    if (channel > 7) return;
    Wire.beginTransmission(_tcaAddr);
    Wire.write(1 << channel);
    Wire.endTransmission();
}

void msc::setLightSource(uint8_t channel, bool on) {
    digitalWrite(_lightSource[channel], on ? LOW : HIGH); // LOW = ON, HIGH = OFF
}

bool msc::checkSensor(uint8_t channel) {
    selectChannel(channel);
    return _as7341.begin();
}

bool msc::begin() {
    Wire.begin();
    for (uint8_t i = 0; i < 3; i++) {
        pinMode(_lightSource[i], OUTPUT);
        digitalWrite(_lightSource[i], HIGH); // Light source OFF
        selectChannel(i);
        if (!_as7341.begin()) {
            Serial.print("AS7341 not found on channel ");
            Serial.println(i);
            return false;
        }
    }
    return true;
}

bool msc::initializeSensor(uint8_t channel) {
    // Only check, do not calibrate
    return checkSensor(channel);
}

AS7341_Data msc::readSensor(uint8_t channel, uint16_t atime, uint16_t astep, as7341_gain_t gain) {
    const int numSamples = 10;
    const int saturationDelay = 80;
    const int interReadDelay = 15;

    AS7341_Data sum = {};
    AS7341_Data data = {};

    selectChannel(channel);
    setLightSource(channel, true);

    // Set sensor config
    _as7341.setATIME(atime);
    _as7341.setASTEP(astep);
    _as7341.setGain(gain);

    delay(saturationDelay);

    for (int i = 0; i < numSamples; i++) {
        if (!_as7341.readAllChannels()) {
            Serial.println("AS7341 read error");
            setLightSource(channel, false);
            data.valid = false;
            return data;
        }
        sum.f1_415nm += _as7341.getChannel(AS7341_CHANNEL_415nm_F1);
        sum.f2_445nm += _as7341.getChannel(AS7341_CHANNEL_445nm_F2);
        sum.f3_480nm += _as7341.getChannel(AS7341_CHANNEL_480nm_F3);
        sum.f4_515nm += _as7341.getChannel(AS7341_CHANNEL_515nm_F4);
        sum.f5_555nm += _as7341.getChannel(AS7341_CHANNEL_555nm_F5);
        sum.f6_590nm += _as7341.getChannel(AS7341_CHANNEL_590nm_F6);
        sum.f7_630nm += _as7341.getChannel(AS7341_CHANNEL_630nm_F7);
        sum.f8_680nm += _as7341.getChannel(AS7341_CHANNEL_680nm_F8);
        sum.clear    += _as7341.getChannel(AS7341_CHANNEL_CLEAR);
        sum.nir      += _as7341.getChannel(AS7341_CHANNEL_NIR);
        delay(interReadDelay);
    }

    data.f1_415nm = sum.f1_415nm / numSamples;
    data.f2_445nm = sum.f2_445nm / numSamples;
    data.f3_480nm = sum.f3_480nm / numSamples;
    data.f4_515nm = sum.f4_515nm / numSamples;
    data.f5_555nm = sum.f5_555nm / numSamples;
    data.f6_590nm = sum.f6_590nm / numSamples;
    data.f7_630nm = sum.f7_630nm / numSamples;
    data.f8_680nm = sum.f8_680nm / numSamples;
    data.clear    = sum.clear    / numSamples;
    data.nir      = sum.nir      / numSamples;
    data.valid = true;

    setLightSource(channel, false);
    return data;
}

void msc::showSensorData(const AS7341_Data& data) {
    if (!data.valid) {
        Serial.println("Sensor data invalid.");
        return;
    }
    Serial.print("415nm: "); Serial.print(data.f1_415nm);
    Serial.print(", 445nm: "); Serial.print(data.f2_445nm);
    Serial.print(", 480nm: "); Serial.print(data.f3_480nm);
    Serial.print(", 515nm: "); Serial.print(data.f4_515nm);
    Serial.print(", 555nm: "); Serial.print(data.f5_555nm);
    Serial.print(", 590nm: "); Serial.print(data.f6_590nm);
    Serial.print(", 630nm: "); Serial.print(data.f7_630nm);
    Serial.print(", 680nm: "); Serial.print(data.f8_680nm);
    Serial.print(", Clear: "); Serial.print(data.clear);
    Serial.print(", NIR: "); Serial.print(data.nir);
    Serial.println();
}

String msc::readSensorJSON(const String& uuid, uint8_t channel, uint16_t atime, uint16_t astep, as7341_gain_t gain) {
    AS7341_Data data = readSensor(channel, atime, astep, gain);

    String json = "{";
    json += "\"uuid\":\"" + uuid + "\",";
    json += "\"channel\":" + String(channel) + ",";
    json += "\"atime\":" + String(atime) + ",";
    json += "\"astep\":" + String(astep) + ",";
    json += "\"gain\":" + String((int)gain) + ",";
    json += "\"valid\":" + String(data.valid ? "true" : "false") + ",";
    json += "\"f1\":" + String(data.f1_415nm) + ",";
    json += "\"f2\":" + String(data.f2_445nm) + ",";
    json += "\"f3\":" + String(data.f3_480nm) + ",";
    json += "\"f4\":" + String(data.f4_515nm) + ",";
    json += "\"f5\":" + String(data.f5_555nm) + ",";
    json += "\"f6\":" + String(data.f6_590nm) + ",";
    json += "\"f7\":" + String(data.f7_630nm) + ",";
    json += "\"f8\":" + String(data.f8_680nm) + ",";
    json += "\"clear\":" + String(data.clear) + ",";
    json += "\"nir\":" + String(data.nir);
    json += "}";

    return json;
}

Adafruit_AS7341& msc::getAS7341() {
    return _as7341;
}
