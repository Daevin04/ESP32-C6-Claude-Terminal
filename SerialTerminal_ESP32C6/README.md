# ESP32-C6 Serial Terminal

A touch-enabled, scrollable serial terminal for ESP32-C6 with LVGL GUI. Designed for bidirectional communication with a Raspberry Pi 5 running `claude_server.py`.

## Features

- Scrollable terminal display (LVGL textarea, 32 KB buffer)
- Touch scroll: top-half touch = scroll up, bottom-half = scroll down
- Hardware button navigation with single/double-click support (OneButton)
- Double-click DOWN button sends `ImageUpload` trigger to Pi
- Incoming serial text displayed in chunked format for long responses
- Black terminal with green text (classic terminal aesthetic)
- All test/debug modes are disabled by default — clean for production use

## Pin Configuration

| Function   | GPIO |
|------------|------|
| LCD SCK    | 1    |
| LCD MOSI   | 2    |
| LCD CS     | 5    |
| LCD DC     | 3    |
| LCD RST    | 4    |
| LCD BL     | 6    |
| I2C SDA    | 8    |
| I2C SCL    | 7    |
| Touch IRQ  | 11   |
| BTN UP     | 7    |
| BTN DOWN   | 18   |
| BTN SEND   | 9    |

## Required Libraries

Install via Arduino Library Manager:

- `Arduino_GFX_Library`
- `lvgl` (v9.x)
- `SensorLib` (provides `TouchDrvCSTXXX.hpp`)
- `OneButton`

## Button Controls

| Button | Action | Result |
|--------|--------|--------|
| UP (GPIO 7) | Single press | Scroll up 30px |
| DOWN (GPIO 18) | Single press | Scroll down 30px |
| DOWN (GPIO 18) | Double-click | Send `ImageUpload` to Pi |
| SEND (GPIO 9) | Single press | Scroll down 60px |

## Serial Commands (from host/Pi)

| Command | Effect |
|---------|--------|
| `START_TEST` | Enable auto test messages |
| `STOP_TEST` | Disable auto test messages |
| `START_SCROLL_TEST` | Enable auto scroll test |
| `STOP_SCROLL_TEST` | Disable auto scroll test |
| `START_PI_SENDER` | Enable periodic Pi sender test |
| `STOP_PI_SENDER` | Disable periodic Pi sender test |
| Any other text | Displayed as `RX: <message>` |

## Display

- Controller: ST7789 IPS
- Resolution: 240 × 280
- Library: Arduino_GFX_Library (HWSPI)

## Touch Panel

- Controller: CST816S (I2C address `0x15`)
- Driver: `TouchDrvCSTXXX` from SensorLib

## Troubleshooting

See `TROUBLESHOOTING.md` for display initialization issues, wiring diagrams, and common fixes.
