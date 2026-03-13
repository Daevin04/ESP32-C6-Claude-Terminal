/*******************************************************************************
 * Touch Panel Configuration for ESP32-C6
 *
 * This file configures the touch panel settings for your ESP32-C6 display.
 * Uncomment and configure the appropriate touch controller for your hardware.
 ******************************************************************************/

#ifndef _TOUCH_H_
#define _TOUCH_H_

// Common touch libraries - uncomment the one you're using
// #include <XPT2046_Touchscreen.h>  // For XPT2046 resistive touch
// #include <FT6X36.h>               // For FT6X36 capacitive touch
// #include <GT911.h>                // For GT911 capacitive touch
// #include <CST816S.h>              // For CST816S capacitive touch

/*******************************************************************************
 * Touch Controller Selection
 * Uncomment ONE of the following sections based on your touch controller
 ******************************************************************************/

// === XPT2046 Resistive Touch Configuration ===
// #define TOUCH_XPT2046
// #define TOUCH_CS 21    // Touch chip select pin
// #define TOUCH_IRQ 22   // Touch interrupt pin (optional)
// XPT2046_Touchscreen touch_xpt(TOUCH_CS, TOUCH_IRQ);

// === FT6X36 Capacitive Touch Configuration ===
// #define TOUCH_FT6X36
// #define TOUCH_SDA 18   // I2C SDA pin
// #define TOUCH_SCL 19   // I2C SCL pin
// #define TOUCH_RST 20   // Touch reset pin (optional)
// #define TOUCH_IRQ 21   // Touch interrupt pin (optional)
// FT6X36 touch_ft6x36(TOUCH_SDA, TOUCH_SCL);

// === GT911 Capacitive Touch Configuration ===
// #define TOUCH_GT911
// #define TOUCH_SDA 18   // I2C SDA pin
// #define TOUCH_SCL 19   // I2C SCL pin
// #define TOUCH_RST 20   // Touch reset pin
// #define TOUCH_IRQ 21   // Touch interrupt pin
// GT911 touch_gt911;

// === CST816S Capacitive Touch Configuration ===
#define TOUCH_CST816S
#define TOUCH_SDA 8    // I2C SDA pin (from your working example)
#define TOUCH_SCL 7    // I2C SCL pin (from your working example)
#define TOUCH_IRQ 11   // Touch interrupt pin (from your working example)
#define CST816_SLAVE_ADDRESS 0x15

// === NO TOUCH (for testing without touch) ===
// #define TOUCH_NONE
// This will use simulated touch events for testing

/*******************************************************************************
 * Touch Variables
 ******************************************************************************/
int16_t touch_last_x = 0, touch_last_y = 0;
int16_t touch_screen_width = 0, touch_screen_height = 0;
uint8_t touch_rotation = 0;
bool touch_pressed = false;
bool touch_released_flag = false;
bool touch_signal = false;

/*******************************************************************************
 * Touch Functions
 ******************************************************************************/

bool touch_init(int16_t w, int16_t h, uint8_t r)
{
  touch_screen_width = w;
  touch_screen_height = h;
  touch_rotation = r;

#ifdef TOUCH_XPT2046
  touch_xpt.begin();
  touch_xpt.setRotation(r);
  Serial.println("XPT2046 touch initialized");
  return true;

#elif defined(TOUCH_FT6X36)
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  if (touch_ft6x36.begin()) {
    Serial.println("FT6X36 touch initialized");
    return true;
  } else {
    Serial.println("FT6X36 touch init failed");
    return false;
  }

#elif defined(TOUCH_GT911)
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  if (TOUCH_RST != -1) {
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_RST, HIGH);
    delay(10);
  }

  if (touch_gt911.begin()) {
    Serial.println("GT911 touch initialized");
    return true;
  } else {
    Serial.println("GT911 touch init failed");
    return false;
  }

#elif defined(TOUCH_CST816S)
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  if (TOUCH_RST != -1) {
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_RST, HIGH);
    delay(10);
  }

  if (touch_cst816s.begin()) {
    Serial.println("CST816S touch initialized");
    return true;
  } else {
    Serial.println("CST816S touch init failed");
    return false;
  }

#else
  // No touch controller - simulation mode
  Serial.println("Touch simulation mode (no hardware)");
  return true;
#endif
}

bool touch_has_signal()
{
  touch_signal = false;

#ifdef TOUCH_XPT2046
  if (touch_xpt.tirqTouched() && touch_xpt.touched()) {
    TS_Point p = touch_xpt.getPoint();

    // Map coordinates based on rotation
    switch (touch_rotation) {
      case 0:
        touch_last_x = map(p.x, 200, 3700, 0, touch_screen_width);
        touch_last_y = map(p.y, 240, 3600, 0, touch_screen_height);
        break;
      case 1:
        touch_last_x = map(p.y, 240, 3600, 0, touch_screen_width);
        touch_last_y = map(p.x, 3700, 200, 0, touch_screen_height);
        break;
      case 2:
        touch_last_x = map(p.x, 3700, 200, 0, touch_screen_width);
        touch_last_y = map(p.y, 3600, 240, 0, touch_screen_height);
        break;
      case 3:
        touch_last_x = map(p.y, 3600, 240, 0, touch_screen_width);
        touch_last_y = map(p.x, 200, 3700, 0, touch_screen_height);
        break;
    }

    touch_signal = true;
    if (!touch_pressed) {
      touch_pressed = true;
      touch_released_flag = false;
    }
  } else if (touch_pressed) {
    touch_pressed = false;
    touch_released_flag = true;
    touch_signal = true;
  }

#elif defined(TOUCH_FT6X36)
  if (touch_ft6x36.touched()) {
    TS_Point p = touch_ft6x36.getPoint();
    touch_last_x = p.x;
    touch_last_y = p.y;
    touch_signal = true;

    if (!touch_pressed) {
      touch_pressed = true;
      touch_released_flag = false;
    }
  } else if (touch_pressed) {
    touch_pressed = false;
    touch_released_flag = true;
    touch_signal = true;
  }

#elif defined(TOUCH_GT911)
  if (touch_gt911.available()) {
    touch_last_x = touch_gt911.readFingerX();
    touch_last_y = touch_gt911.readFingerY();
    touch_signal = true;

    if (!touch_pressed) {
      touch_pressed = true;
      touch_released_flag = false;
    }
  } else if (touch_pressed) {
    touch_pressed = false;
    touch_released_flag = true;
    touch_signal = true;
  }

#elif defined(TOUCH_CST816S)
  if (touch_cst816s.available()) {
    touch_last_x = touch_cst816s.data.x;
    touch_last_y = touch_cst816s.data.y;
    touch_signal = true;

    if (!touch_pressed) {
      touch_pressed = true;
      touch_released_flag = false;
    }
  } else if (touch_pressed) {
    touch_pressed = false;
    touch_released_flag = true;
    touch_signal = true;
  }

#else
  // Simulation mode - no actual touch events
  // You can add simulated touch events here for testing
  touch_signal = false;
#endif

  return touch_signal;
}

bool touch_touched()
{
  return touch_pressed && !touch_released_flag;
}

bool touch_released()
{
  if (touch_released_flag) {
    touch_released_flag = false; // Clear the flag
    return true;
  }
  return false;
}

/*******************************************************************************
 * Touch Calibration (for resistive touch panels)
 *
 * If you need to calibrate your touch panel, you can adjust the mapping
 * values in the touch_has_signal() function above.
 *
 * To calibrate:
 * 1. Enable touch debugging by uncommenting the lines below
 * 2. Touch the corners and edges of your screen
 * 3. Note the raw values and adjust the map() function parameters
 ******************************************************************************/

void touch_debug_print()
{
#ifdef DEBUG_TOUCH
  if (touch_has_signal()) {
    Serial.print("Touch: x=");
    Serial.print(touch_last_x);
    Serial.print(" y=");
    Serial.print(touch_last_y);
    Serial.print(" pressed=");
    Serial.println(touch_pressed);
  }
#endif
}

#endif // _TOUCH_H_