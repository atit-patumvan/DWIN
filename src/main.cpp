#include <Arduino.h>
#include "msc.h"

#define BUTTON_PIN 0

msc sensor;

const as7341_gain_t gains[] = {
    AS7341_GAIN_0_5X,
    AS7341_GAIN_1X,
    AS7341_GAIN_2X,
    AS7341_GAIN_4X,
    AS7341_GAIN_8X,
    AS7341_GAIN_16X,
    AS7341_GAIN_32X,
    AS7341_GAIN_64X,
    AS7341_GAIN_128X,
    AS7341_GAIN_256X
};

const char* gainLabels[] = {
    "0.5X", "1X", "2X", "4X", "8X", "16X", "32X", "64X", "128X", "256X"
};

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("AS7341 Multi-sensor (NO Calibration), Gain Sweep Per Channel, JSON Output");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    if (!sensor.begin()) {
        Serial.println("Sensor initialization failed!");
        while (1);
    }
    Serial.println("All sensors initialized and ready.");
    Serial.println("Press GPIO0 button to sweep gains for each channel and print JSON.");
}

void loop() {
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(BUTTON_PIN);

    if (lastButtonState == HIGH && buttonState == LOW) { // Button just pressed
        Serial.println("\nButton pressed: Sweeping gains per channel and outputting JSON...");

        String uuid = "abcd-1234-efgh-5678"; // Example uuid, replace with your own if needed

        for (uint8_t i = 0; i < 3; i++) {
            Serial.print("=== Channel "); Serial.print(i); Serial.println(" ===");
            for (size_t g = 0; g < sizeof(gains) / sizeof(gains[0]); ++g) {
                Serial.print("  Gain: "); Serial.println(gainLabels[g]);
                String jsonResult = sensor.readSensorJSON(uuid, i, 100, 999, gains[g]);
                Serial.println(jsonResult);
                delay(300);
            }
            Serial.println();
        }

        // Wait for button release
        while (digitalRead(BUTTON_PIN) == LOW) {
            delay(10);
        }
        Serial.println("Done. Press again to sweep gains for each channel.");
    }
    lastButtonState = buttonState;
    delay(10);
}
