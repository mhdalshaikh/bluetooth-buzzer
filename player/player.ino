/*
 * Bluetooth Buzzer System - Player Device
 * Hardware: RF-Nano (Arduino Nano + built-in nRF24L01)
 *
 * IMPORTANT: Set PLAYER_ID before uploading to each player device!
 */

#include <SPI.h>
#include <RF24.h>

// ============================================
// CONFIGURE THIS FOR EACH PLAYER
// ============================================
#define PLAYER_ID  1  // Change this: 0=Green, 1=Red, 2=Blue, etc.

// ============================================
// Configuration for RF-Nano
// ============================================
#define RF_CE_PIN       10
#define RF_CSN_PIN      9
#define RF_CHANNEL      100
#define RF_PA_LEVEL     RF24_PA_HIGH

// Player pins
#define PLAYER_BUTTON_PIN   7   // Button signal
#define PLAYER_BUZZER_PIN   4   // Buzzer
#define PLAYER_LED_PIN      2   // Button's built-in LED

// Timing
#define RESPONSE_TIMEOUT    2000   // 2 sec to wait for referee
#define RF_RETRY_DELAY      5
#define RF_RETRY_COUNT      15

// Protocol message types
#define MSG_RESET       0x01
#define MSG_BUZZ        0x02
#define MSG_ACK_WINNER  0x03
#define MSG_LOCKOUT     0x04

// Pipe addresses
const byte REFEREE_ADDR[6] = "REF01";
const byte BROADCAST_ADDR[6] = "BCAST";

// Message structure
struct BuzzerMessage {
    uint8_t type;
    uint8_t playerId;
    uint8_t data;
};

// ============================================
// RF24 Setup
// ============================================
RF24 radio(RF_CE_PIN, RF_CSN_PIN);

// ============================================
// State Variables
// ============================================
bool canBuzz = true;
bool isWinner = false;
bool waitingForResponse = false;
bool lastButtonState = HIGH;
unsigned long lastPressTime = 0;
unsigned long buzzSentTime = 0;

// ============================================
// Setup
// ============================================
void setup() {
    Serial.begin(115200);
    Serial.print(F("Player "));
    Serial.print(PLAYER_ID);
    Serial.println(F(" starting..."));

    pinMode(PLAYER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PLAYER_BUZZER_PIN, OUTPUT);
    pinMode(PLAYER_LED_PIN, OUTPUT);

    digitalWrite(PLAYER_BUZZER_PIN, LOW);
    digitalWrite(PLAYER_LED_PIN, LOW);

    radio.begin();
    delay(100);

    radio.setChannel(RF_CHANNEL);
    if (radio.getChannel() != RF_CHANNEL) {
        Serial.println(F("Radio init failed!"));
        while (1) {
            digitalWrite(PLAYER_LED_PIN, HIGH);
            delay(100);
            digitalWrite(PLAYER_LED_PIN, LOW);
            delay(100);
        }
    }
    Serial.println(F("Radio OK!"));

    radio.setPALevel(RF_PA_LEVEL);
    radio.setDataRate(RF24_250KBPS);
    radio.setRetries(RF_RETRY_DELAY, RF_RETRY_COUNT);

    radio.openWritingPipe(REFEREE_ADDR);
    radio.openReadingPipe(1, BROADCAST_ADDR);

    radio.startListening();

    beep(100);
    blinkLED(3);

    Serial.println(F("Ready!"));
}

// ============================================
// Main Loop
// ============================================
void loop() {
    checkRadio();
    checkButton();
    checkTimeout();
}

// ============================================
// Check for incoming radio messages
// ============================================
void checkRadio() {
    if (radio.available()) {
        BuzzerMessage msg;
        radio.read(&msg, sizeof(msg));

        switch (msg.type) {
            case MSG_RESET:
                handleReset();
                break;

            case MSG_ACK_WINNER:
                if (msg.playerId == PLAYER_ID) {
                    handleWin();
                }
                break;

            case MSG_LOCKOUT:
                if (msg.playerId != PLAYER_ID) {
                    handleLockout();
                }
                break;
        }
    }
}

// ============================================
// Check button
// ============================================
void checkButton() {
    bool reading = digitalRead(PLAYER_BUTTON_PIN);

    if (reading == LOW && lastButtonState == HIGH) {
        if ((millis() - lastPressTime) > 200) {
            lastPressTime = millis();

            if (canBuzz && !isWinner) {
                sendBuzz();
            }
        }
    }

    lastButtonState = reading;
}

// ============================================
// Check for response timeout
// ============================================
void checkTimeout() {
    if (waitingForResponse && !isWinner) {
        if ((millis() - buzzSentTime) > RESPONSE_TIMEOUT) {
            Serial.println(F("No response - ready again"));
            waitingForResponse = false;
            canBuzz = true;
        }
    }
}

// ============================================
// Send buzz message
// ============================================
void sendBuzz() {
    Serial.println(F("BUZZ!"));
    beep(50);

    radio.stopListening();

    BuzzerMessage msg;
    msg.type = MSG_BUZZ;
    msg.playerId = PLAYER_ID;
    msg.data = 0;

    bool success = radio.write(&msg, sizeof(msg));

    radio.startListening();

    if (success) {
        Serial.println(F("Buzz sent - waiting..."));
        waitingForResponse = true;
        buzzSentTime = millis();
    } else {
        Serial.println(F("Send failed - try again"));
    }

    canBuzz = false;
}

// ============================================
// Handle reset
// ============================================
void handleReset() {
    Serial.println(F("RESET - new round!"));

    isWinner = false;
    waitingForResponse = false;
    digitalWrite(PLAYER_LED_PIN, LOW);
    beep(30);
    delay(100);
    canBuzz = true;
}

// ============================================
// Handle win
// ============================================
void handleWin() {
    Serial.println(F("*** WINNER! ***"));

    isWinner = true;
    canBuzz = false;
    waitingForResponse = false;
    digitalWrite(PLAYER_LED_PIN, HIGH);

    beep(150);
}

// ============================================
// Handle lockout
// ============================================
void handleLockout() {
    Serial.println(F("Locked out"));

    canBuzz = false;
    isWinner = false;
    waitingForResponse = false;
    digitalWrite(PLAYER_LED_PIN, LOW);
}

// ============================================
// Utility: Beep
// ============================================
void beep(int duration) {
    digitalWrite(PLAYER_BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(PLAYER_BUZZER_PIN, LOW);
}

// ============================================
// Utility: Blink LED
// ============================================
void blinkLED(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(PLAYER_LED_PIN, HIGH);
        delay(100);
        digitalWrite(PLAYER_LED_PIN, LOW);
        delay(100);
    }
}
