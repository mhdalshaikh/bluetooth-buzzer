/*
 * Button test - check D7 and D2
 */

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("\nButton Test - watching D7 and D2"));
    Serial.println(F("Press the button...\n"));

    pinMode(2, INPUT_PULLUP);
    pinMode(7, INPUT_PULLUP);
}

void loop() {
    static bool lastD2 = HIGH;
    static bool lastD7 = HIGH;

    bool d2 = digitalRead(2);
    bool d7 = digitalRead(7);

    if (d2 != lastD2 || d7 != lastD7) {
        Serial.print(F("D2="));
        Serial.print(d2 ? "HIGH" : "LOW");
        Serial.print(F("  D7="));
        Serial.println(d7 ? "HIGH" : "LOW");

        lastD2 = d2;
        lastD7 = d7;
    }

    delay(10);
}
