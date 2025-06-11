#include "dwin.h"
#include <cstdio>   // for sprintf

// Constructor
Dwin::Dwin(uint8_t rxPin, uint8_t txPin) : _rxPin(rxPin), _txPin(txPin), _uiPage(0) {}

// Utility: float to 4-byte hex (DWIN format)
void Dwin::floatToHex(float f, byte* hex) {
    byte* f_byte = reinterpret_cast<byte*>(&f);
    memcpy(hex, f_byte, 4);
    for (int i = 0, j = 3; i < 2; i++, j--) { // Reverse
        byte temp = hex[i];
        hex[i] = hex[j];
        hex[j] = temp;
    }
}

// Send 4-byte float value to DWIN
void Dwin::sendFloat(uint16_t vp, float f) {
    byte data[4] = {0};
    byte open[10] = {0x5A, 0xA5, 0x07, 0x82, 0x12, 0x00, 0x3f, 0x80, 0x00, 0x00};
    open[4] = highByte(vp);
    open[5] = lowByte(vp);
    floatToHex(f, data);
    for (int i = 0; i < 4; i++) {
        open[6 + i] = data[i];
    }
    Serial2.write(open, sizeof(open));
    delay(50);
    while (Serial2.available()) {
        int inhex = Serial2.read();
        if (inhex == 0x5A || inhex == 0xA5) continue;
    }
}

void Dwin::sendInt16(uint16_t vp, int16_t value) {
    // Frame format:
    // [0] 0x5A
    // [1] 0xA5
    // [2] LL   = number of bytes from CMD to last data byte (6 here)
    // [3] CMD  = 0x82 (write variable)
    // [4] SUB  = 0x11 (write integer)
    // [5] VP_H = high byte of vp
    // [6] VP_L = low  byte of vp
    // [7] DAT_H = high byte of value
    // [8] DAT_L = low  byte of value

    byte frame[8];
    frame[0] = 0x5A;
    frame[1] = 0xA5;
    frame[2] = 0x05;              // length = CMD + SUB + ADDRH + ADDRL + DATAH + DATAL = 6 bytes
    frame[3] = 0x82;              // write command
    frame[4] = highByte(vp);      // VP address high
    frame[5] = lowByte(vp);       // VP address low
    frame[6] = highByte(value);   // data high
    frame[7] = lowByte(value);    // data low

    Serial2.write(frame, sizeof(frame));
    delay(20);

    // Drain any ACK bytes (0x5A/0xA5) so next read isn’t confused
    while (Serial2.available()) {
      int b = Serial2.read();
      if (b == 0x5A || b == 0xA5) continue;
    }

    Serial.println("Data send");
}

// Send curve data
void Dwin::sendCurve(byte number) {
    byte Frame[14] = {0x5A, 0xA5, 0x0B, 0x82, 0x03, 0x10, 0x5A, 0xA5, 0x01, 0x00, 0x00, 0x01, 0x00, number};
    Serial2.write(Frame, sizeof(Frame));
    while (Serial2.available()) {
        int inhex = Serial2.read();
        if (inhex == 0x5A || inhex == 0xA5) continue;
    }
    delay(100);
}

// Clear curve data
void Dwin::clearCurve() {
    byte Frame[8] = {0x5A, 0xA5, 0x05, 0x82, 0x03, 0x01, 0x00, 0x00};
    Serial2.write(Frame, sizeof(Frame));
    while (Serial2.available()) {
        int inhex = Serial2.read();
        if (inhex == 0x5A || inhex == 0xA5) continue;
    }
}

// Change display page
void Dwin::pageSwitch(byte pageNo) {
    byte open[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, pageNo};
    Serial2.write(open, sizeof(open));
    delay(10);
    String response = "";
    while (Serial2.available()) {
        int inhex = Serial2.read();
        if (inhex == 0x5A || inhex == 0xA5) continue;
        for (int i = 1; i < inhex; i++) {
            if (i == 1) Serial2.read();
        }
    }
    Serial.println("Page Switch : " + String(pageNo) + " -> " + response);
    delay(10);
}

// Initialize DWIN serial and display
void Dwin::init() {
    Serial2.begin(9600, SERIAL_8N1, _rxPin, _txPin);
    delay(1000);
    sendFloat(0x6000, 0.00);
    pageSwitch(0);
}

// Listener: read and parse a single event from DWIN
DwinEvent Dwin::listen() {
    DwinEvent event = {false, 0, 0, 0};
    int startAdd = 0, endAdd = 0, dataVal = 0;

    if (Serial2.available()) {
        int inhex = Serial2.read();
        if (inhex == 0x5A || inhex == 0xA5) return event;
        for (int i = 1; i <= inhex; i++) {
            while (!Serial2.available());
            int incomingByte = Serial2.read();
            if (i == 2) startAdd = incomingByte;
            if (i == 3) endAdd = incomingByte;
            if (i == inhex) dataVal = incomingByte;
        }
        event.valid = true;
        event.startAddr = startAdd;
        event.endAddr = endAdd;
        event.dataVal = dataVal;
    }
    return event;
}

// Merge startAddr and endAddr to 16-bit address
uint16_t Dwin::mergeAddr(int startAddr, int endAddr) {
    return ((startAddr & 0xFF) << 8) | (endAddr & 0xFF);
}

// Getter for current UI page
uint8_t Dwin::getUiPage() const {
    return _uiPage;
}

String Dwin::getTextData(uint16_t address, uint8_t length, uint16_t timeout) {
    byte cmd[7] = {
        0x5A, 0xA5, 0x04, 0x83,
        uint8_t(address >> 8),
        uint8_t(address & 0xFF),
        length
    };

    // Flush any old data
    while (Serial2.available()) Serial2.read();

    Serial2.write(cmd, sizeof(cmd));

    unsigned long start = millis();
    String result = "";
    bool foundHeader = false;
    int expectedBytes = length + 2; // CMD + ADDR_H + ADDR_L + data length

    while (millis() - start < timeout) {
        if (!foundHeader && Serial2.available() >= 3) {
            // Look for 0x5A 0xA5
            if (Serial2.read() == 0x5A && Serial2.read() == 0xA5) {
                uint8_t frameLen = Serial2.read(); // should be length + 3
                foundHeader = true;
            }
        }
        if (foundHeader && Serial2.available() >= expectedBytes) {
            // Skip CMD, ADDR_H, ADDR_L
            Serial2.read(); Serial2.read(); Serial2.read();
            // Read only up to first 0xFF
            for (int i = 0; i < length; i++) {
                char c = Serial2.read();
                if (c == (char)0xFF) break;
                result += c;
            }
            break;
        }
    }

    // **Drain any leftover bytes** so next listen() isn't confused
    while (Serial2.available()) Serial2.read();

    return result;
}


// Turn buzzer on or off (default VP address 0x0084)
void Dwin::setBuzzer(bool on) {
    uint16_t buzzerVP = 0x0084; // Change if your buzzer is mapped elsewhere
    byte cmd[8] = {
        0x5A, 0xA5, 0x05, 0x82,
        (uint8_t)(buzzerVP >> 8), (uint8_t)(buzzerVP & 0xFF),
        0x00, (uint8_t)(on ? 1 : 0)
    };
    Serial2.write(cmd, sizeof(cmd));
    delay(20);
    while (Serial2.available()) {
        int inhex = Serial2.read();
        if (inhex == 0x5A || inhex == 0xA5) continue;
    }
}

String Dwin::toHex16(uint16_t value) {
    char buf[7];               // "0x" + 4 hex digits + '\0'
    sprintf(buf, "0x%04X", value);
    return String(buf);
}

void Dwin::writeString(uint16_t vp, const String& text, uint8_t maxLen) {
    // ---- 1. Header ----
    uint8_t frameLen = 3 + maxLen; // CMD + ADDRH + ADDRL + DATA(maxLen)
    byte header[6] = {
        0x5A, 0xA5,
        frameLen,
        0x82,                   // write-variable command
        uint8_t(vp >> 8),       // VP high byte
        uint8_t(vp & 0xFF)      // VP low byte
    };
    Serial2.write(header, sizeof(header));

    // ---- 2. Payload ----
    uint8_t actualLen = min((uint8_t)text.length(), maxLen);
    Serial2.write((const uint8_t*)text.c_str(), actualLen);

    // ---- 3. Padding ----
    for (uint8_t i = actualLen; i < maxLen; i++) {
        Serial2.write(0xFF);
    }

    // ---- 4. Sync ----
    delay(20);
    while (Serial2.available()) {
        int b = Serial2.read();
        if (b == 0x5A || b == 0xA5) continue;
    }
}