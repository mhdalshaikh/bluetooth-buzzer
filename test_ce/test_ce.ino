/*
 * Find CE pin for RF-Nano
 * We know CSN=9 works, now find CE
 */

#include <SPI.h>
#include <RF24.h>

// CSN is confirmed as D9
#define CSN_PIN 9

// CE pins to try
const int cePins[] = {10, 8, 7, 6, 5, 4, 3, 2, A0, A1, A2, A3, A4, A5};
const int numPins = sizeof(cePins) / sizeof(cePins[0]);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("\n\nCE Pin Finder (CSN=9 confirmed)"));
    Serial.println(F("================================\n"));

    for (int i = 0; i < numPins; i++) {
        int ce = cePins[i];

        Serial.print(F("Testing CE="));
        if (ce >= A0) {
            Serial.print(F("A"));
            Serial.print(ce - A0);
        } else {
            Serial.print(F("D"));
            Serial.print(ce);
        }
        Serial.print(F(", CSN=D9 ... "));

        RF24 radio(ce, CSN_PIN);
        delay(50);

        if (radio.begin()) {
            Serial.println(F("SUCCESS!"));
            Serial.println(F("\n*** WORKING CONFIG ***"));
            Serial.print(F("CE="));
            if (ce >= A0) {
                Serial.print(F("A"));
                Serial.println(ce - A0);
            } else {
                Serial.print(F("D"));
                Serial.println(ce);
            }
            Serial.println(F("CSN=D9"));
            Serial.println(F("\nRadio details:"));
            radio.printDetails();
            return;
        } else {
            Serial.println(F("failed"));
        }
        delay(50);
    }

    Serial.println(F("\nNo CE pin found. The CE line might not be connected."));
}

void loop() {
}
