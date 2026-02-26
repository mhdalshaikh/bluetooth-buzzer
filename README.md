# Bluetooth Buzzer System

A wireless quiz buzzer system using Arduino Nano and nRF24L01 modules. Supports multiple teams with a referee controller.

## Features

- **Scalable**: Supports up to 8 teams/players (easily extendable)
- **Fair**: First-to-press wins with RF-based timing
- **Visual feedback**: RGB LED on referee shows winning team's color
- **Audio feedback**: Buzzers on all devices
- **Auto-reset**: Automatically resets after 3 seconds (configurable)
- **Manual reset**: Referee can reset anytime

## System Overview

```
┌─────────────┐         ┌─────────────┐
│   REFEREE   │◄───────►│  PLAYER 0   │
│  (Master)   │    RF   │   (Green)   │
│             │◄───────►├─────────────┤
│  - RGB LED  │         │  PLAYER 1   │
│  - Button   │◄───────►│   (Red)     │
│  - Buzzer   │         ├─────────────┤
│             │◄───────►│  PLAYER N   │
└─────────────┘         │   (...)     │
                        └─────────────┘
```

## Hardware Required

### Per Player Device
- 1x Arduino Nano
- 1x nRF24L01 module (or nRF24L01+PA+LNA for longer range)
- 1x LED (any color, or use team-specific color)
- 1x Push button
- 1x Active buzzer (5V)
- 1x 10µF capacitor (for nRF24 power stabilization)
- Jumper wires

### Referee Device
- 1x Arduino Nano
- 1x nRF24L01 module
- 1x WS2812 RGB LED (NeoPixel)
- 1x Push button
- 1x Active buzzer (5V)
- 1x 10µF capacitor
- Jumper wires

## Wiring Diagrams

### nRF24L01 Module (Common for all devices)

```
nRF24L01 Pinout:
┌─────────────────┐
│  1 GND   2 VCC  │  VCC = 3.3V (IMPORTANT!)
│  3 CE    4 CSN  │
│  5 SCK   6 MOSI │
│  7 MISO  8 IRQ  │  IRQ not used
└─────────────────┘

Connection to Arduino Nano:
┌──────────────┬─────────────┐
│ nRF24L01     │ Arduino     │
├──────────────┼─────────────┤
│ GND          │ GND         │
│ VCC          │ 3.3V        │
│ CE           │ D8          │
│ CSN          │ D10         │
│ SCK          │ D13         │
│ MOSI         │ D11         │
│ MISO         │ D12         │
└──────────────┴─────────────┘

IMPORTANT: Add 10µF capacitor between VCC and GND
           close to the nRF24L01 module!
```

### Player Device Wiring

```
Arduino Nano
     ┌────────────────────────┐
     │    ┌──────────────┐    │
 D13─┤SCK │              │    │
 D12─┤MISO│   nRF24L01   │    │
 D11─┤MOSI│              │    │
 D10─┤CSN └──────────────┘    │
  D9─┤───── BUTTON ─────┬─ GND│
  D8─┤CE                │     │
  D7─┤───── LED ────────┼─ GND│ (with 220Ω resistor)
  D4─┤───── BUZZER ─────┴─ GND│
     │                        │
 3V3─┤─── nRF24 VCC           │
 GND─┤─── Common GND          │
     └────────────────────────┘

Button: Connect between D9 and GND (uses internal pullup)
LED: Connect anode to D7 (through 220Ω), cathode to GND
Buzzer: Connect + to D4, - to GND
```

### Referee Device Wiring

```
Arduino Nano
     ┌────────────────────────┐
     │    ┌──────────────┐    │
 D13─┤SCK │              │    │
 D12─┤MISO│   nRF24L01   │    │
 D11─┤MOSI│              │    │
 D10─┤CSN └──────────────┘    │
  D8─┤CE                      │
  D7─┤───── BUTTON ─────┬─ GND│
  D4─┤───── BUZZER ─────┼─ GND│
  D3─┤───── WS2812 DIN  │     │
     │                  │     │
 5V ─┤───── WS2812 VCC  │     │
 3V3─┤─── nRF24 VCC     │     │
 GND─┤─── Common GND ───┴─────│
     └────────────────────────┘

Button: Connect between D7 and GND
Buzzer: Connect + to D4, - to GND
WS2812: VCC to 5V, GND to GND, DIN to D3
```

## Software Setup

### Required Libraries

Install these via Arduino Library Manager:

1. **RF24** by TMRh20
2. **Adafruit NeoPixel** (for referee only)

### Uploading Code

#### Player Devices

1. Open `player/player.ino`
2. **IMPORTANT**: Change `PLAYER_ID` on line 13 for each player:
   ```cpp
   #define PLAYER_ID  0  // Player 0 = Green
   #define PLAYER_ID  1  // Player 1 = Red
   #define PLAYER_ID  2  // Player 2 = Blue
   // etc.
   ```
3. Upload to Arduino

#### Referee Device

1. Open `referee/referee.ino`
2. Upload to Arduino (no configuration needed)

## Usage

1. Power on all devices
2. Wait for startup sequence (LED blinks, buzzer beeps)
3. **Referee presses button** to start a round
4. All players can now buzz
5. First player to press wins:
   - Winner's LED lights up
   - Winner's buzzer beeps 3 times
   - Referee's RGB shows winner's color
6. System auto-resets after 3 seconds, OR
7. Referee can press button to reset early

## Team Colors

| Player ID | Color   | RGB Value       |
|-----------|---------|-----------------|
| 0         | Green   | (0, 255, 0)     |
| 1         | Red     | (255, 0, 0)     |
| 2         | Blue    | (0, 0, 255)     |
| 3         | Yellow  | (255, 255, 0)   |
| 4         | Magenta | (255, 0, 255)   |
| 5         | Cyan    | (0, 255, 255)   |
| 6         | Orange  | (255, 128, 0)   |
| 7         | White   | (255, 255, 255) |

## Configuration

Edit `common/config.h` to customize:

- **RF_CHANNEL**: Change if interference (0-125)
- **LOCKOUT_DURATION**: Time before auto-reset (default: 3000ms)
- **DEBOUNCE_DELAY**: Button debounce time (default: 50ms)
- **TEAM_COLORS**: Add/modify team colors

## Troubleshooting

### Radio not initializing
- Check 3.3V power to nRF24L01
- Add 10µF capacitor near the module
- Check SPI wiring (SCK, MOSI, MISO)

### Poor range
- Use nRF24L01+PA+LNA modules
- Add external antenna
- Reduce RF_PA_LEVEL if too close (interference)

### Button not responding
- Check button wiring (should be between pin and GND)
- Verify INPUT_PULLUP is working

### Players not receiving reset
- Ensure all devices use same RF_CHANNEL
- Check BROADCAST_ADDR matches in config.h

## Scaling Up

To add more players:

1. Build another player device
2. Set unique `PLAYER_ID` before uploading
3. (Optional) Add new color to `TEAM_COLORS` array

No changes needed to referee code!

## License

MIT License - Feel free to modify and share!
