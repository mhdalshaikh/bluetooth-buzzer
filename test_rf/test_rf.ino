/*
 * RF-Nano Pin Finder
 * Tests different CE/CSN combinations to find working config
 */

#include <SPI.h>
#include <RF24.h>

// Pin combinations to try
const int pinConfigs[][2] = {
    {10, 9},   // CE=10, CSN=9
    {9, 10},   // CE=9, CSN=10
    {7, 8},    // CE=7, CSN=8
    {8, 7},    // CE=8, CSN=7
    {10, 8},   // CE=10, CSN=8
    {9, 8},    // CE=9, CSN=8
};

const int numConfigs = sizeof(pinConfigs) / sizeof(pinConfigs[0]);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("\n\nRF-Nano Pin Finder"));
    Serial.println(F("==================\n"));

    for (int i = 0; i < numConfigs; i++) {
        int ce = pinConfigs[i][0];
        int csn = pinConfigs[i][1];

        Serial.print(F("Testing CE="));
        Serial.print(ce);
        Serial.print(F(", CSN="));
        Serial.print(csn);
        Serial.print(F(" ... "));

        RF24 radio(ce, csn);
        delay(100);

        if (radio.begin()) {
            Serial.println(F("SUCCESS!"));
            Serial.println(F("\n*** WORKING CONFIG FOUND ***"));
            Serial.print(F("Use: CE="));
            Serial.print(ce);
            Serial.print(F(", CSN="));
            Serial.println(csn);

            // Print more info
            radio.printDetails();
            return;
        } else {
            Serial.println(F("failed"));
        }
        delay(100);
    }

    Serial.println(F("\nNo working configuration found."));
    Serial.println(F("Check if your board has RF module properly soldered."));
}

void loop() {
    // Nothing
}
