/*
 * Force RF24 initialization
 * Some RF-Nano boards have CE tied high internally
 */

#include <SPI.h>
#include <RF24.h>

// CE might be unused/tied high, but we still need to pass something
// CSN confirmed as D9
RF24 radio(10, 9);  // CE=10 (might be ignored), CSN=9

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("\n\nForced RF24 Init Test"));
    Serial.println(F("=====================\n"));

    // Don't check begin() return value
    radio.begin();
    delay(100);

    // Manually verify communication by reading a register
    Serial.print(F("Checking communication... "));

    // Try to write and read back a test value
    radio.setChannel(123);
    delay(10);
    uint8_t channel = cycleChannel();

    Serial.print(F("Channel set to 123, read back: "));
    Serial.println(channel);

    if (channel == 123) {
        Serial.println(F("\n*** RADIO IS WORKING! ***\n"));

        // Configure for our buzzer system
        radio.setChannel(100);
        radio.setPALevel(RF24_PA_HIGH);
        radio.setDataRate(RF24_250KBPS);

        Serial.println(F("Configuration:"));
        radio.printDetails();

        Serial.println(F("\n\nUse these settings in your code:"));
        Serial.println(F("#define RF_CE_PIN  10"));
        Serial.println(F("#define RF_CSN_PIN 9"));
        Serial.println(F("\nAnd IGNORE the begin() return value!"));
    } else {
        Serial.println(F("\nCommunication failed."));
    }
}

uint8_t cycleChannel() {
    // Read channel via SPI directly
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    digitalWrite(9, LOW);
    SPI.transfer(0x05);  // Read RF_CH register
    uint8_t ch = SPI.transfer(0xFF);
    digitalWrite(9, HIGH);
    SPI.endTransaction();
    return ch;
}

void loop() {
    // Blink LED to show we're running
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
        lastBlink = millis();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}
