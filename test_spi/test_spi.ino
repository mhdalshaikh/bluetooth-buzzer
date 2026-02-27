/*
 * Direct SPI test for nRF24L01
 * Tries to read the STATUS register directly
 */

#include <SPI.h>

// Try different CSN pins
const int csnPins[] = {9, 10, 8, 7, 6, 5, 4, 3, 2};
const int numPins = sizeof(csnPins) / sizeof(csnPins[0]);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("\n\nDirect SPI Test for nRF24L01"));
    Serial.println(F("============================\n"));

    SPI.begin();

    for (int i = 0; i < numPins; i++) {
        int csn = csnPins[i];

        pinMode(csn, OUTPUT);
        digitalWrite(csn, HIGH);
        delay(10);

        Serial.print(F("Testing CSN=D"));
        Serial.print(csn);
        Serial.print(F(" ... "));

        // Try to read STATUS register (command 0xFF = NOP, returns status)
        digitalWrite(csn, LOW);
        delayMicroseconds(10);
        uint8_t status = SPI.transfer(0xFF);
        digitalWrite(csn, HIGH);

        Serial.print(F("Status=0x"));
        if (status < 16) Serial.print(F("0"));
        Serial.print(status, HEX);

        // Valid nRF24L01 status is usually 0x0E at startup
        // 0x00 or 0xFF typically means no communication
        if (status != 0x00 && status != 0xFF) {
            Serial.println(F(" <- VALID RESPONSE!"));
        } else {
            Serial.println(F(" (no response)"));
        }

        digitalWrite(csn, HIGH);
        delay(50);
    }

    Serial.println(F("\nIf you see a valid response (not 0x00 or 0xFF),"));
    Serial.println(F("that CSN pin is connected to the nRF24L01."));
    Serial.println(F("\nExpected startup status: 0x0E"));
}

void loop() {
    // Nothing
}
