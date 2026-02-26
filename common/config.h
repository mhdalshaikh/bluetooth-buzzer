/*
 * Bluetooth Buzzer System - Configuration
 * Common configuration for all devices
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// RF24 Configuration (nRF24L01)
// ============================================
// nRF24L01 uses hardware SPI: SCK=D13, MOSI=D11, MISO=D12
#define RF_CE_PIN       8   // Chip Enable
#define RF_CSN_PIN      10  // Chip Select

#define RF_CHANNEL      100 // RF channel (0-125)
#define RF_PA_LEVEL     RF24_PA_HIGH // Power level

// Pipe addresses for communication
const byte REFEREE_ADDR[6] = "REF01";  // Referee listening address
const byte BROADCAST_ADDR[6] = "BCAST"; // Broadcast to all players

// ============================================
// Player Configuration
// ============================================
#define PLAYER_BUTTON_PIN   9
#define PLAYER_BUZZER_PIN   4
#define PLAYER_LED_PIN      7

// ============================================
// Referee Configuration
// ============================================
#define REFEREE_BUTTON_PIN  7
#define REFEREE_BUZZER_PIN  4
#define REFEREE_RGB_PIN     3   // WS2812 data pin (moved from D11/D12 due to SPI conflict)

#define REFEREE_NUM_LEDS    1   // Number of WS2812 LEDs

// ============================================
// Timing Configuration (milliseconds)
// ============================================
#define LOCKOUT_DURATION    3000    // 3 seconds lockout after winner
#define DEBOUNCE_DELAY      50      // Button debounce time
#define BUZZER_DURATION     200     // Buzzer beep duration
#define RF_RETRY_DELAY      5       // Delay between retries
#define RF_RETRY_COUNT      15      // Number of retries

// ============================================
// Team/Player Definitions (Scalable)
// ============================================
// Add more teams by extending this array
// Format: {R, G, B} for each team

#define MAX_PLAYERS 8  // Maximum supported players

// Team colors (RGB values for WS2812)
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

// ============================================
// Protocol Message Types
// ============================================
#define MSG_RESET       0x01  // Referee broadcast: reset all
#define MSG_BUZZ        0x02  // Player -> Referee: button pressed
#define MSG_ACK_WINNER  0x03  // Referee -> Player: you won
#define MSG_LOCKOUT     0x04  // Referee broadcast: someone else won

// ============================================
// Message Structure
// ============================================
struct BuzzerMessage {
    uint8_t type;       // Message type (MSG_*)
    uint8_t playerId;   // Player ID (0-7)
    uint8_t data;       // Optional data
};

#endif // CONFIG_H
