/*
 * Bluetooth Buzzer System - Player Device
 *
 * Hardware: Arduino Nano + nRF24L01
 *
 * IMPORTANT: Set PLAYER_ID before uploading to each player device!
 */

#include <SPI.h>
#include <RF24.h>

// ============================================
// CONFIGURE THIS FOR EACH PLAYER
// ============================================
#define PLAYER_ID  0  // Change this: 0=Green, 1=Red, 2=Blue, etc.

// ============================================
// Configuration
// ============================================

// RF24 pins (nRF24L01 uses hardware SPI: SCK=D13, MOSI=D11, MISO=D12)
#define RF_CE_PIN       8
#define RF_CSN_PIN      10
#define RF_CHANNEL      100
#define RF_PA_LEVEL     RF24_PA_HIGH

// Player pins
#define PLAYER_BUTTON_PIN   9
#define PLAYER_BUZZER_PIN   4
#define PLAYER_LED_PIN      7

// Timing
#define DEBOUNCE_DELAY      50
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

// Player-specific listening address
byte playerAddr[6];

// ============================================
// State Variables
// ============================================
bool canBuzz = false;
bool isWinner = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// ============================================
// Setup
// ============================================
void setup() {
    Serial.begin(115200);
    Serial.print(F("Player "));
    Serial.print(PLAYER_ID);
    Serial.println(F(" starting..."));

    // Setup pins
    pinMode(PLAYER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PLAYER_BUZZER_PIN, OUTPUT);
    pinMode(PLAYER_LED_PIN, OUTPUT);

    digitalWrite(PLAYER_BUZZER_PIN, LOW);
    digitalWrite(PLAYER_LED_PIN, LOW);

    // Create player-specific address
    memcpy(playerAddr, "PLY00", 6);
    playerAddr[3] = '0' + PLAYER_ID;

    // Initialize radio
    if (!radio.begin()) {
        Serial.println(F("Radio init failed!"));
        while (1) {
            digitalWrite(PLAYER_LED_PIN, HIGH);
            delay(100);
            digitalWrite(PLAYER_LED_PIN, LOW);
            delay(100);
        }
    }

    radio.setChannel(RF_CHANNEL);
    radio.setPALevel(RF_PA_LEVEL);
    radio.setDataRate(RF24_250KBPS);
    radio.setRetries(RF_RETRY_DELAY, RF_RETRY_COUNT);

    // Open pipes
    radio.openWritingPipe(REFEREE_ADDR);
    radio.openReadingPipe(1, BROADCAST_ADDR);
    radio.openReadingPipe(2, playerAddr);

    radio.startListening();

    // Startup indication
    beep(100);
    blinkLED(3);

    canBuzz = false;
    Serial.println(F("Ready. Waiting for referee reset..."));
}

// ============================================
// Main Loop
// ============================================
void loop() {
    checkRadio();
    checkButton();
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
// Check button with debounce
// ============================================
void checkButton() {
    bool reading = digitalRead(PLAYER_BUTTON_PIN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading == LOW && lastButtonState == HIGH) {
            if (canBuzz && !isWinner) {
                sendBuzz();
            }
        }
    }

    lastButtonState = reading;
}

// ============================================
// Send buzz message to referee
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
        Serial.println(F("Buzz sent!"));
    } else {
        Serial.println(F("Buzz send failed!"));
    }

    canBuzz = false;
}

// ============================================
// Handle reset from referee
// ============================================
void handleReset() {
    Serial.println(F("RESET received"));

    canBuzz = true;
    isWinner = false;
    digitalWrite(PLAYER_LED_PIN, LOW);
    beep(30);
}

// ============================================
// Handle winning acknowledgment
// ============================================
void handleWin() {
    Serial.println(F("*** WINNER! ***"));

    isWinner = true;
    canBuzz = false;
    digitalWrite(PLAYER_LED_PIN, HIGH);

    for (int i = 0; i < 3; i++) {
        beep(100);
        delay(100);
    }
}

// ============================================
// Handle lockout (someone else won)
// ============================================
void handleLockout() {
    Serial.println(F("Locked out - another player won"));

    canBuzz = false;
    isWinner = false;
    digitalWrite(PLAYER_LED_PIN, LOW);
}

// ============================================
// Utility: Beep the buzzer
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
