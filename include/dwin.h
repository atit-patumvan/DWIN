#ifndef DWIN_H
#define DWIN_H

#include <Arduino.h>

struct DwinEvent {
    bool valid;
    int startAddr;
    int endAddr;
    int dataVal;
};

class Dwin {
public:
    Dwin(uint8_t rxPin = 16, uint8_t txPin = 17);

    // Core initialization
    void init();

    // DGUS protocol communication
    void sendFloat(uint16_t vp, float f);
    void sendInt(byte number);
    void sendCurve(byte number);
    void clearCurve();
    void pageSwitch(byte pageNo);

    // Listener/Response
    DwinEvent listen();

    // Helper and utility
    static uint16_t mergeAddr(int startAddr, int endAddr);
    uint8_t getUiPage() const;

    // Advanced: Text and Buzzer control
    String getTextData(uint16_t address, uint8_t length, uint16_t timeout = 500);
    void setBuzzer(bool on);
    // Helper: format a 16-bit value as "0x0000"
    static String toHex16(uint16_t value);

private:
    uint8_t _rxPin, _txPin;
    uint8_t _uiPage;
    void floatToHex(float f, byte* hex);
};

#endif // DWIN_H
