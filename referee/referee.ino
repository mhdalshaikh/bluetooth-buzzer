/*
 * Bluetooth Buzzer System - Referee Controller
 * Hardware: RF-Nano (Arduino Nano + built-in nRF24L01)
 */

#include <SPI.h>
#include <RF24.h>
#include <Adafruit_NeoPixel.h>

// ============================================
// Configuration for RF-Nano
// ============================================
#define RF_CE_PIN       10
#define RF_CSN_PIN      9
#define RF_CHANNEL      100
#define RF_PA_LEVEL     RF24_PA_HIGH

// Referee pins
#define REFEREE_BUTTON_PIN  7
#define REFEREE_BUZZER_PIN  6
#define REFEREE_RGB_PIN     5
#define REFEREE_NUM_LEDS    1

// Timing
#define LOCKOUT_DURATION    3000  // 3 seconds before auto-reset
#define RF_RETRY_DELAY      5
#define RF_RETRY_COUNT      15

// Max players
#define MAX_PLAYERS 8

// Protocol message types
#define MSG_RESET       0x01
#define MSG_BUZZ        0x02
#define MSG_ACK_WINNER  0x03
#define MSG_LOCKOUT     0x04

// Pipe addresses
const byte REFEREE_ADDR[6] = "REF01";
const byte BROADCAST_ADDR[6] = "BCAST";

// Team colors (RGB)
const uint8_t TEAM_COLORS[][3] = {
    {0,   255, 0},    // Player 0: Green
    {255, 0,   0},    // Player 1: Red
    {0,   0,   255},  // Player 2: Blue
    {255, 255, 0},    // Player 3: Yellow
    {255, 0,   255},  // Player 4: Magenta
    {0,   255, 255},  // Player 5: Cyan
    {255, 128, 0},    // Player 6: Orange
    {255, 255, 255},  // Player 7: White
};

// Message structure
struct BuzzerMessage {
    uint8_t type;
    uint8_t playerId;
    uint8_t data;
};

// ============================================
// Hardware Setup
// ============================================
RF24 radio(RF_CE_PIN, RF_CSN_PIN);
Adafruit_NeoPixel rgbLed(REFEREE_NUM_LEDS, REFEREE_RGB_PIN, NEO_RGB + NEO_KHZ800);

// ============================================
// State Variables
// ============================================
enum RefereeState {
    STATE_READY,
    STATE_LOCKED
};

RefereeState currentState = STATE_READY;
int8_t winnerId = -1;
unsigned long lockoutStartTime = 0;
bool lastButtonState = HIGH;
unsigned long lastPressTime = 0;

// ============================================
// Setup
// ============================================
void setup() {
    Serial.begin(115200);
    Serial.println(F("Referee starting..."));

    pinMode(REFEREE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(REFEREE_BUZZER_PIN, OUTPUT);
    digitalWrite(REFEREE_BUZZER_PIN, LOW);

    rgbLed.begin();
    rgbLed.setBrightness(128);
    setColor(0, 0, 0);

    radio.begin();
    delay(100);

    radio.setChannel(RF_CHANNEL);
    if (radio.getChannel() != RF_CHANNEL) {
        Serial.println(F("Radio init failed!"));
        while (1) {
            setColor(255, 0, 0);
            delay(100);
            setColor(0, 0, 0);
            delay(100);
        }
    }
    Serial.println(F("Radio OK!"));

    radio.setPALevel(RF_PA_LEVEL);
    radio.setDataRate(RF24_250KBPS);
    radio.setRetries(RF_RETRY_DELAY, RF_RETRY_COUNT);

    radio.openReadingPipe(1, REFEREE_ADDR);
    radio.startListening();

    startupAnimation();
    Serial.println(F("Ready!"));
}

// ============================================
// Main Loop
// ============================================
void loop() {
    checkButton();

    switch (currentState) {
        case STATE_READY:
            checkForBuzz();
            break;

        case STATE_LOCKED:
            checkLockoutTimeout();
            break;
    }
}

// ============================================
// Check referee button
// ============================================
void checkButton() {
    bool reading = digitalRead(REFEREE_BUTTON_PIN);

    if (reading == LOW && lastButtonState == HIGH) {
        if ((millis() - lastPressTime) > 200) {
            lastPressTime = millis();
            broadcastReset();
        }
    }

    lastButtonState = reading;
}

// ============================================
// Broadcast reset
// ============================================
void broadcastReset() {
    Serial.println(F("=== NEW ROUND ==="));

    winnerId = -1;
    currentState = STATE_READY;
    setColor(0, 0, 0);
    beep(100);

    radio.stopListening();
    radio.openWritingPipe(BROADCAST_ADDR);

    BuzzerMessage msg;
    msg.type = MSG_RESET;
    msg.playerId = 0xFF;
    msg.data = 0;

    for (int i = 0; i < 5; i++) {
        radio.write(&msg, sizeof(msg));
        delay(10);
    }
    delay(50);

    radio.openReadingPipe(1, REFEREE_ADDR);
    radio.startListening();
    delay(50);

    Serial.println(F("Listening..."));
}

// ============================================
// Check for buzz
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
// Handle winner
// ============================================
void handleWinner(uint8_t playerId) {
    Serial.print(F("*** WINNER: Player "));
    Serial.print(playerId);
    Serial.println(F(" ***"));

    winnerId = playerId;
    currentState = STATE_LOCKED;
    lockoutStartTime = millis();

    setColor(TEAM_COLORS[playerId][0], TEAM_COLORS[playerId][1], TEAM_COLORS[playerId][2]);

    sendAckToWinner(playerId);
    sendLockout(playerId);

    beep(100);
}

// ============================================
// Send ACK
// ============================================
void sendAckToWinner(uint8_t playerId) {
    radio.stopListening();
    radio.openWritingPipe(BROADCAST_ADDR);

    BuzzerMessage msg;
    msg.type = MSG_ACK_WINNER;
    msg.playerId = playerId;
    msg.data = 0;

    for (int i = 0; i < 3; i++) {
        radio.write(&msg, sizeof(msg));
        delay(3);
    }

    radio.startListening();
}

// ============================================
// Send lockout
// ============================================
void sendLockout(uint8_t winId) {
    radio.stopListening();
    radio.openWritingPipe(BROADCAST_ADDR);

    BuzzerMessage msg;
    msg.type = MSG_LOCKOUT;
    msg.playerId = winId;
    msg.data = 0;

    for (int i = 0; i < 3; i++) {
        radio.write(&msg, sizeof(msg));
        delay(3);
    }

    radio.openReadingPipe(1, REFEREE_ADDR);
    radio.startListening();
}

// ============================================
// Check lockout timeout
// ============================================
void checkLockoutTimeout() {
    if ((millis() - lockoutStartTime) >= LOCKOUT_DURATION) {
        Serial.println(F("Auto-reset"));
        broadcastReset();
    }
}

// ============================================
// Utility: Set RGB color
// ============================================
void setColor(uint8_t r, uint8_t g, uint8_t b) {
    rgbLed.setPixelColor(0, rgbLed.Color(r, g, b));
    rgbLed.show();
}

// ============================================
// Utility: Beep
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
    for (int i = 0; i < 4; i++) {
        setColor(TEAM_COLORS[i][0], TEAM_COLORS[i][1], TEAM_COLORS[i][2]);
        delay(150);
    }
    setColor(0, 0, 0);
    beep(100);
}
