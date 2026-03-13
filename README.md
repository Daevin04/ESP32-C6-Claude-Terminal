# ESP32-C6 Claude Terminal

An embedded AI assistant system that pairs an **ESP32-C6 touchscreen terminal** with a **Raspberry Pi 5 vision server** powered by the Claude API. Point the Pi camera at any question or text, press a button on the ESP32, and receive an AI-generated answer displayed directly on the ESP32's screen.

---

## System Overview

```
┌─────────────────────────────┐        ┌──────────────────────────────┐
│        ESP32-C6             │        │       Raspberry Pi 5         │
│                             │        │                              │
│  ST7789 240×280 Display     │        │  Arducam IMX708 Camera       │
│  CST816S Touch Panel        │◄──────►│  Flask REST Server           │
│  LVGL Scrollable Terminal   │ Serial │  Claude Vision API           │
│  3× Hardware Buttons        │        │  Image → Q&A Pipeline        │
└─────────────────────────────┘        └──────────────────────────────┘

**Workflow:**
1. Double-click the DOWN button on ESP32-C6 → sends `ImageUpload` over serial
2. Pi 5 receives command → captures image with camera
3. Image is sent directly to Claude Vision API
4. Claude analyzes the image and returns a structured answer
5. Answer is streamed back over serial and displayed on the ESP32 terminal

---

## Repository Structure

```
![IMG_0266](https://github.com/user-attachments/assets/1e113b5e-16ad-47ba-ba9f-2a7359cf5aae)
![IMG_0264](https://github.com/user-attachments/assets/7ed380c4-51e4-4f27-aa73-e6a03e9fb2c3)


ESP32-C6-Claude-Terminal/
├── SerialTerminal_ESP32C6/          # Main ESP32-C6 Arduino sketch
│   ├── SerialTerminal_ESP32C6.ino   # Primary sketch file
│   ├── touch.h                      # Touch panel configuration
│   ├── lv_conf.h                    # LVGL configuration
│   ├── README.md                    # Hardware setup guide
│   ├── TROUBLESHOOTING.md           # Display & wiring debug guide
│   └── backup/                      # Previous sketch versions
│
├── claude_server.py                 # Raspberry Pi 5 Flask vision server
│
├── Raspb5/                          # Raspberry Pi 5 helper scripts
│   ├── main.py                      # Entry point
│   ├── test_chatgpt.py              # OpenAI GPT-4o vision listener (serial)
│   ├── chatgpt_bot.py               # ChatGPT Selenium bot skeleton
│   ├── chatgpt_login.py             # Google login automation helper
│   ├── web_scraper.py               # ChatGPT web scraper (via Selenium)
│   ├── web_scraper_backup.py        # Web scraper backup version
│   └── requirements.txt             # Python dependencies
│
└── examples/                        # Reference Arduino sketches
    ├── 01_audio_out/                 # ES8311 audio codec example
    ├── 02_button_example/            # Hardware button input
    ├── 03_battery_example/           # Battery voltage reading
    ├── 04_es8311_example/            # ES8311 full example
    ├── 05_gfx_helloworld/            # Basic GFX display test
    ├── 06_gfx_pdq_graphicstest/      # PDQ graphics benchmark
    ├── 07_gfx_clock/                 # Analog clock display
    ├── 08_gfx_u8g2_font/             # U8G2 font rendering
    ├── 09_gfx_image/                 # JPEG image rendering
    ├── 10_esp_wifi_analyzer/         # Wi-Fi channel analyzer
    ├── 11_pcf85063_example/          # RTC module example
    ├── 12_qmi8658_example/           # IMU accelerometer example
    ├── 13_lvgl_arduino_v8/           # LVGL v8 example
    ├── 14_lvgl_arduino_v9/           # LVGL v9 example
    └── 15_touch_example/             # CST816S touch panel example
```

> **Note:** The `libraries/` directory is excluded from this repo. Install all required libraries via the Arduino Library Manager (see [Software Requirements](#software-requirements) below).

---

## Hardware

### ESP32-C6 Side

| Component | Details |
|-----------|---------|
| MCU | ESP32-C6 Dev Module |
| Display | 1.69" ST7789 IPS, 240×280 px |
| Touch | CST816S capacitive touch panel |
| Buttons | 3× tactile push buttons |
| Interface | USB Serial (115200 baud) |

#### Pin Configuration

| Function | GPIO |
|----------|------|
| LCD SCK  | 1    |
| LCD MOSI | 2    |
| LCD CS   | 5    |
| LCD DC   | 3    |
| LCD RST  | 4    |
| LCD BL   | 6    |
| I2C SDA  | 8    |
| I2C SCL  | 7    |
| Touch IRQ| 11   |
| BTN UP   | 7    |
| BTN DOWN | 18   |
| BTN SEND | 9    |

### Raspberry Pi 5 Side

| Component | Details |
|-----------|---------|
| SBC | Raspberry Pi 5 |
| Camera | Arducam IMX708 Wide (or compatible `rpicam-still` camera) |
| Connection | USB serial to ESP32-C6 |
| OS | Raspberry Pi OS (64-bit) |

---

## Software Requirements

### ESP32-C6 (Arduino IDE)

Install the following libraries via **Arduino Library Manager** (`Sketch → Include Library → Manage Libraries`):

| Library | Purpose |
|---------|---------|
| `Arduino_GFX_Library` | ST7789 display driver |
| `lvgl` (v9.x) | GUI framework |
| `SensorLib` (or `TouchDrvCSTXXX`) | CST816S touch driver |
| `OneButton` | Advanced button handling (single/double click) |

**Board:** Install the ESP32 board package and select `ESP32C6 Dev Module`.

### Raspberry Pi 5

Install Python dependencies:

```bash
pip install flask requests
```

Set your Claude API key as an environment variable:

```bash
export CLAUDE_API_KEY=your_api_key_here
```

Get an API key at [console.anthropic.com](https://console.anthropic.com).

---

## Setup & Installation

### 1. ESP32-C6 Firmware

1. Open `SerialTerminal_ESP32C6/SerialTerminal_ESP32C6.ino` in Arduino IDE
2. Install all required libraries listed above
3. Select `Tools → Board → ESP32C6 Dev Module`
4. Select the correct COM port
5. Upload the sketch

See `SerialTerminal_ESP32C6/TROUBLESHOOTING.md` if your display stays blank after upload.

### 2. Raspberry Pi 5 Server

1. Clone this repo onto the Pi:
   ```bash
   git clone https://github.com/Daevin04/ESP32-C6-Claude-Terminal.git
   cd ESP32-C6-Claude-Terminal
   ```

2. Set your API key:
   ```bash
   export CLAUDE_API_KEY=your_api_key_here
   ```

3. Start the server:
   ```bash
   python claude_server.py
   ```

4. Verify it's running by visiting `http://<pi-ip>:5000` in a browser or checking the `/status` endpoint.

### 3. Connect ESP32-C6 to Pi

Connect the ESP32-C6 to the Pi via USB. The Pi will see it as `/dev/ttyACM0` or `/dev/ttyUSB0`.

---

## Usage

### Terminal Controls

| Action | Result |
|--------|--------|
| BTN UP (single press) | Scroll terminal up |
| BTN DOWN (single press) | Scroll terminal down |
| BTN DOWN (double-click) | Send `ImageUpload` command to Pi |
| Serial input (`START_TEST`) | Enable auto test messages |
| Serial input (`STOP_TEST`) | Disable auto test messages |

### REST API (Pi Server)

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Server info page |
| `/status` | GET | Camera + API health check |
| `/analyze_photo` | POST | Capture image and return Claude analysis |

---

## Configuration

### Changing the Claude Model

In `claude_server.py`, update the model field:

```python
"model": "claude-opus-4-6",   # Most capable
"model": "claude-sonnet-4-6", # Balanced (default)
"model": "claude-haiku-4-5-20251001",  # Fastest
```

### Adjusting the Terminal Buffer

In `SerialTerminal_ESP32C6.ino`:

```cpp
const int MAX_BUFFER_SIZE = 30000;  // Increase for more message history
```

### Customizing the System Prompt

The analysis prompt in `claude_server.py` inside `analyze_image_with_claude()` can be replaced with any domain-specific instructions.

---

## Troubleshooting

| Issue | Fix |
|-------|-----|
| Blank display | See `SerialTerminal_ESP32C6/TROUBLESHOOTING.md` |
| Touch not working | Check CST816S I2C wiring (SDA=8, SCL=7) |
| No serial data on Pi | Verify `/dev/ttyACM0`, check baud rate is 115200 |
| Claude API error | Check `CLAUDE_API_KEY` env var is set correctly |
| Camera not found | Run `rpicam-still --list-cameras` to verify camera |

---

## License

MIT License — free to use, modify, and distribute.
