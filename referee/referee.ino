/*
 * Bluetooth Buzzer System - Referee Controller
 *
 * Hardware: Arduino Nano + nRF24L01 + WS2812 RGB LED
 *
 * The referee acts as the master controller:
 * - Broadcasts RESET to enable all players
 * - Receives buzz signals and determines winner
 * - Shows winner's team color on RGB LED
 * - Auto-resets after lockout duration
 */

#include <SPI.h>
#include <RF24.h>
#include <Adafruit_NeoPixel.h>
#include "../common/config.h"

// ============================================
// RF24 Setup
// ============================================
RF24 radio(RF_CE_PIN, RF_CSN_PIN);

// ============================================
// NeoPixel Setup
// ============================================
Adafruit_NeoPixel rgbLed(REFEREE_NUM_LEDS, REFEREE_RGB_PIN, NEO_GRB + NEO_KHZ800);

// ============================================
// State Variables
// ============================================
enum RefereeState {
    STATE_IDLE,         // Waiting for reset button
    STATE_LISTENING,    // Listening for player buzzes
    STATE_LOCKED        // Winner determined, locked out
};

RefereeState currentState = STATE_IDLE;
int8_t winnerId = -1;
unsigned long lockoutStartTime = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// ============================================
// Setup
// ============================================
void setup() {
    Serial.begin(115200);
    Serial.println(F("Referee Controller starting..."));

    // Setup pins
    pinMode(REFEREE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(REFEREE_BUZZER_PIN, OUTPUT);

    digitalWrite(REFEREE_BUZZER_PIN, LOW);

    // Initialize NeoPixel
    rgbLed.begin();
    rgbLed.setBrightness(128);  // 50% brightness
    rgbLed.clear();
    rgbLed.show();

    // Initialize radio
    if (!radio.begin()) {
        Serial.println(F("Radio init failed!"));
        while (1) {
            // Flash red to indicate error
            setColor(255, 0, 0);
            delay(100);
            setColor(0, 0, 0);
            delay(100);
        }
    }

    radio.setChannel(RF_CHANNEL);
    radio.setPALevel(RF_PA_LEVEL);
    radio.setDataRate(RF24_250KBPS);
    radio.setRetries(RF_RETRY_DELAY, RF_RETRY_COUNT);

    // Open pipes
    radio.openReadingPipe(1, REFEREE_ADDR);  // Listen for player buzzes

    radio.startListening();

    // Startup indication
    startupAnimation();

    Serial.println(F("Ready. Press button to start round."));
}

// ============================================
// Main Loop
// ============================================
void loop() {
    checkButton();

    switch (currentState) {
        case STATE_IDLE:
            // Waiting for referee to press reset
            break;

        case STATE_LISTENING:
            checkForBuzz();
            break;

        case STATE_LOCKED:
            checkLockoutTimeout();
            break;
    }
}

// ============================================
// Check referee button with debounce
// ============================================
void checkButton() {
    bool reading = digitalRead(REFEREE_BUTTON_PIN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading == LOW && lastButtonState == HIGH) {
            // Button pressed
            broadcastReset();
        }
    }

    lastButtonState = reading;
}

// ============================================
// Broadcast reset to all players
// ============================================
void broadcastReset() {
    Serial.println(F("=== ROUND START ==="));

    // Reset state
    winnerId = -1;
    currentState = STATE_LISTENING;

    // Clear LED
    setColor(0, 0, 0);

    // Beep
    beep(100);

    // Send reset broadcast
    radio.stopListening();
    radio.openWritingPipe(BROADCAST_ADDR);

    BuzzerMessage msg;
    msg.type = MSG_RESET;
    msg.playerId = 0xFF;  // Broadcast indicator
    msg.data = 0;

    // Send multiple times for reliability
    for (int i = 0; i < 3; i++) {
        radio.write(&msg, sizeof(msg));
        delay(5);
    }

    // Resume listening for buzzes
    radio.openReadingPipe(1, REFEREE_ADDR);
    radio.startListening();

    Serial.println(F("Listening for buzzes..."));

    // Flash white briefly to indicate ready
    setColor(255, 255, 255);
    delay(100);
    setColor(0, 0, 0);
}

// ============================================
// Check for incoming buzz signals
// ============================================
void checkForBuzz() {
    if (radio.available()) {
        BuzzerMessage msg;
        radio.read(&msg, sizeof(msg));

        if (msg.type == MSG_BUZZ && msg.playerId < MAX_PLAYERS) {
            handleWinner(msg.playerId);
        }
    }
}

// ============================================
// Handle winner determination
// ============================================
void handleWinner(uint8_t playerId) {
    Serial.print(F("*** WINNER: Player "));
    Serial.print(playerId);
    Serial.println(F(" ***"));

    winnerId = playerId;
    currentState = STATE_LOCKED;
    lockoutStartTime = millis();

    // Show winner's color
    setColor(
        TEAM_COLORS[playerId][0],
        TEAM_COLORS[playerId][1],
        TEAM_COLORS[playerId][2]
    );

    // Victory beep
    beep(200);

    // Send acknowledgment to winner
    sendAckToWinner(playerId);

    // Send lockout to all others
    sendLockout(playerId);
}

// ============================================
// Send ACK to winning player
// ============================================
void sendAckToWinner(uint8_t playerId) {
    radio.stopListening();

    // Create player-specific address
    byte playerAddr[6];
    memcpy(playerAddr, "PLY00", 6);
    playerAddr[3] = '0' + playerId;

    radio.openWritingPipe(playerAddr);

    BuzzerMessage msg;
    msg.type = MSG_ACK_WINNER;
    msg.playerId = playerId;
    msg.data = 0;

    // Send multiple times
    for (int i = 0; i < 3; i++) {
        radio.write(&msg, sizeof(msg));
        delay(5);
    }

    radio.startListening();
}

// ============================================
// Send lockout broadcast to all players
// ============================================
void sendLockout(uint8_t winnerId) {
    radio.stopListening();
    radio.openWritingPipe(BROADCAST_ADDR);

    BuzzerMessage msg;
    msg.type = MSG_LOCKOUT;
    msg.playerId = winnerId;  // Tell them who won
    msg.data = 0;

    // Send multiple times
    for (int i = 0; i < 3; i++) {
        radio.write(&msg, sizeof(msg));
        delay(5);
    }

    radio.openReadingPipe(1, REFEREE_ADDR);
    radio.startListening();
}

// ============================================
// Check for lockout timeout (auto-reset)
// ============================================
void checkLockoutTimeout() {
    if ((millis() - lockoutStartTime) >= LOCKOUT_DURATION) {
        Serial.println(F("Auto-reset after timeout"));
        broadcastReset();
    }
}

// ============================================
// Utility: Set RGB LED color
// ============================================
void setColor(uint8_t r, uint8_t g, uint8_t b) {
    rgbLed.setPixelColor(0, rgbLed.Color(r, g, b));
    rgbLed.show();
}

// ============================================
// Utility: Beep the buzzer
// ============================================
void beep(int duration) {
    digitalWrite(REFEREE_BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(REFEREE_BUZZER_PIN, LOW);
}

// ============================================
// Startup animation
// ============================================
void startupAnimation() {
    // Cycle through team colors
    for (int i = 0; i < 4; i++) {
        setColor(
            TEAM_COLORS[i][0],
            TEAM_COLORS[i][1],
            TEAM_COLORS[i][2]
        );
        delay(200);
    }
    setColor(0, 0, 0);
    beep(100);
}
