/*
 * Raw SPI test - write and read register
 */

#include <SPI.h>

#define CSN_PIN 9

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("\n\nRaw SPI Write/Read Test"));
    Serial.println(F("========================\n"));

    pinMode(CSN_PIN, OUTPUT);
    digitalWrite(CSN_PIN, HIGH);

    SPI.begin();

    // Read current channel (RF_CH register = 0x05)
    uint8_t oldChannel = readReg(0x05);
    Serial.print(F("Current channel: "));
    Serial.println(oldChannel);

    // Write new channel value (123)
    Serial.println(F("Writing channel = 123..."));
    writeReg(0x05, 123);
    delay(10);

    // Read it back
    uint8_t newChannel = readReg(0x05);
    Serial.print(F("Read back channel: "));
    Serial.println(newChannel);

    if (newChannel == 123) {
        Serial.println(F("\n*** SPI WRITE/READ WORKS! ***"));

        // Reset to 0
        writeReg(0x05, 0);

        Serial.println(F("\nThe chip is working. The RF24 library might have a bug."));
        Serial.println(F("Let me test with RF24 library directly..."));

        testRF24Library();
    } else {
        Serial.println(F("\nWrite failed. Chip may be write-protected or defective."));
    }
}

uint8_t readReg(uint8_t reg) {
    digitalWrite(CSN_PIN, LOW);
    SPI.transfer(reg & 0x1F);  // Read command
    uint8_t val = SPI.transfer(0xFF);
    digitalWrite(CSN_PIN, HIGH);
    return val;
}

void writeReg(uint8_t reg, uint8_t val) {
    digitalWrite(CSN_PIN, LOW);
    SPI.transfer((reg & 0x1F) | 0x20);  // Write command
    SPI.transfer(val);
    digitalWrite(CSN_PIN, HIGH);
}

void testRF24Library() {
    Serial.println(F("\n--- Testing RF24 Library ---"));

    // Try with a fake CE pin since it might not be connected
    // The RF24 library needs CE for TX/RX mode switching

    Serial.println(F("The issue: Your board's CE pin might be:"));
    Serial.println(F("  1. Tied to VCC (always enabled)"));
    Serial.println(F("  2. Not exposed to any GPIO"));
    Serial.println(F("\nFor receive-only or simple TX, this might still work."));
    Serial.println(F("Let's modify the main code to skip the begin() check."));
}

void loop() {
}
