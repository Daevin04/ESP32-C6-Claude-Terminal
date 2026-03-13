# ESP32-C6 Display Troubleshooting Guide

## Issue: Blank Display Screen

Your ESP32-C6 uploads successfully but the 1.69" display shows nothing. Here's how to fix it:

## Step 1: Test with Simple Display Code

**Upload `DisplayTest_Simple.ino` first** - this tests basic display functionality without LVGL complexity.

### Expected Results:
- ✅ **Serial Monitor shows**: "Display initialized successfully!"
- ✅ **Screen shows**: Color cycling (red, green, blue, black)
- ✅ **Text appears**: "ESP32-C6 Display Test"
- ✅ **LED blinks**: Built-in LED should blink every second

### If Still Blank:

## Step 2: Check Hardware Connections

### Pin Connections for ESP32-C6:
```
Display Pin  →  ESP32-C6 Pin
VCC         →   3.3V
GND         →   GND
CS          →   GPIO 7
DC (A0)     →   GPIO 2
RST         →   GPIO 1
SDA (MOSI)  →   GPIO 6
SCL (SCLK)  →   GPIO 4
BL (LED)    →   GPIO 3
```

### Common Issues:
- ❌ **Loose connections** - Check all jumper wires
- ❌ **Wrong voltage** - 1.69" displays usually need 3.3V, not 5V
- ❌ **Bad power supply** - ESP32-C6 needs adequate power (500mA+)
- ❌ **Damaged display** - Try with another display if available

## Step 3: Try Different Display Configurations

In `DisplayTest_Simple.ino`, try these options one by one:

### Option 1: ST7789 240x280 (Most Common)
```cpp
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0, true, 240, 280, 0, 20);
```

### Option 2: ST7789 240x240
```cpp
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0, true, 240, 240, 0, 0);
```

### Option 3: Different Rotation
```cpp
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 1, true, 240, 280, 0, 20);
//                                                    ^ Try 0, 1, 2, or 3
```

### Option 4: Non-IPS Version
```cpp
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0, false, 240, 280, 0, 20);
//                                                       ^ false instead of true
```

## Step 4: Check Serial Monitor Output

Open Serial Monitor (115200 baud) and look for:

### Good Output:
```
=== ESP32-C6 Display Test ===
Pin Configuration: CS: 7, DC: 2, RST: 1, BL: 3
Backlight: ON
Display initialized successfully!
Display size: 240 x 280
Test 1: Color fill test
```

### Bad Output:
```
ERROR: Display initialization failed!
Check wiring and power connections
```

## Step 5: Pin Configuration Issues

If your 1.69" display uses different pins, modify these in the code:

```cpp
// Check your display board pinout and adjust:
#define TFT_CS    7    // Chip Select
#define TFT_DC    2    // Data/Command
#define TFT_RST   1    // Reset
#define TFT_SCLK  4    // Clock
#define TFT_MOSI  6    // Data
#define GFX_BL    3    // Backlight
```

## Step 6: Common 1.69" Display Types

### Type A: ST7789 240x280
```cpp
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0, true, 240, 280, 0, 20);
```

### Type B: ST7789 240x240
```cpp
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0, true, 240, 240, 0, 0);
```

### Type C: Round Display
```cpp
Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST, 0, true);
```

## Step 7: Power and Signal Issues

### Check Power:
- **Voltage**: Measure 3.3V at display VCC pin
- **Current**: ESP32-C6 + display needs 200-500mA
- **USB Cable**: Try different cable (some are power-only)

### Check Signals:
- **SPI Speed**: Try slower SPI if available
- **Wire Length**: Keep wires short (< 10cm)
- **Breadboard**: Try direct soldering if using breadboard

## Step 8: Advanced Debugging

### Enable Debug Output:
```cpp
// Add to setup()
Serial.setDebugOutput(true);
```

### Test Individual Pins:
```cpp
// Add to setup() to test backlight
digitalWrite(GFX_BL, HIGH);  // Should light up display
delay(1000);
digitalWrite(GFX_BL, LOW);   // Should dim display
```

## Quick Fixes to Try:

1. **Different USB Port** - Try another USB port
2. **External Power** - Power ESP32-C6 with external 5V supply
3. **Fresh Upload** - Restart Arduino IDE and re-upload
4. **Basic Example** - Try Arduino_GFX library examples first
5. **Display Rotation** - Try all 4 rotations (0, 1, 2, 3)

## If Nothing Works:

### Test with Multimeter:
- **3.3V at VCC pin**
- **0V at GND pin**
- **Backlight voltage** at BL pin when HIGH

### Try Known-Good Code:
Use Arduino_GFX library examples:
- `File → Examples → GFX Library for Arduino → HelloWorld`
- Modify pins to match your setup

### Hardware Issues:
- **Dead display** - Try another display
- **Wrong display type** - Verify controller chip (ST7789, ILI9341, etc.)
- **Incompatible board** - Some displays need level shifters

## Success Checklist:

✅ Serial Monitor shows successful initialization
✅ Built-in LED is blinking
✅ Display shows colors during test
✅ Text appears on screen
✅ Backlight is working

Once the simple test works, you can go back to the full LVGL terminal code!