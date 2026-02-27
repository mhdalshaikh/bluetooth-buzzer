/*
 * Detailed SPI diagnostic
 */

#include <SPI.h>

#define CSN_PIN 9

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("\n\nDetailed SPI Diagnostic"));
    Serial.println(F("========================\n"));

    pinMode(CSN_PIN, OUTPUT);
    digitalWrite(CSN_PIN, HIGH);

    // Try different SPI speeds
    Serial.println(F("Testing different SPI speeds...\n"));

    uint32_t speeds[] = {1000000, 4000000, 8000000, 250000};
    const char* speedNames[] = {"1MHz", "4MHz", "8MHz", "250KHz"};

    for (int s = 0; s < 4; s++) {
        Serial.print(F("Speed: "));
        Serial.println(speedNames[s]);

        SPI.begin();
        SPI.beginTransaction(SPISettings(speeds[s], MSBFIRST, SPI_MODE0));

        // Read status
        uint8_t status = readReg(0x07);
        Serial.print(F("  Status reg: 0x"));
        Serial.println(status, HEX);

        // Read config
        uint8_t config = readReg(0x00);
        Serial.print(F("  Config reg: 0x"));
        Serial.println(config, HEX);

        // Try to write config (enable CRC, power up)
        Serial.println(F("  Writing config=0x0E..."));
        writeReg(0x00, 0x0E);
        delay(5);

        uint8_t configNew = readReg(0x00);
        Serial.print(F("  Config now: 0x"));
        Serial.println(configNew, HEX);

        if (configNew == 0x0E) {
            Serial.println(F("  *** WRITE SUCCESS! ***"));
        }

        // Try writing to RF_CH
        Serial.println(F("  Writing channel=99..."));
        writeReg(0x05, 99);
        delay(5);

        uint8_t ch = readReg(0x05);
        Serial.print(F("  Channel now: "));
        Serial.println(ch);

        if (ch == 99) {
            Serial.println(F("  *** CHANNEL WRITE SUCCESS! ***"));
        }

        SPI.endTransaction();
        Serial.println();
        delay(100);
    }

    Serial.println(F("\n--- Checking SPI pins ---"));
    Serial.println(F("MOSI should be D11"));
    Serial.println(F("MISO should be D12"));
    Serial.println(F("SCK should be D13"));
    Serial.println(F("\nDo you have anything connected to D11, D12, or D13?"));
}

uint8_t readReg(uint8_t reg) {
    digitalWrite(CSN_PIN, LOW);
    delayMicroseconds(5);
    SPI.transfer(reg & 0x1F);
    uint8_t val = SPI.transfer(0xFF);
    digitalWrite(CSN_PIN, HIGH);
    delayMicroseconds(5);
    return val;
}

void writeReg(uint8_t reg, uint8_t val) {
    digitalWrite(CSN_PIN, LOW);
    delayMicroseconds(5);
    SPI.transfer((reg & 0x1F) | 0x20);
    SPI.transfer(val);
    digitalWrite(CSN_PIN, HIGH);
    delayMicroseconds(5);
}

void loop() {
}
