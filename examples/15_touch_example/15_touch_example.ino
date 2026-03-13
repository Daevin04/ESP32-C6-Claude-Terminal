#include <Arduino_GFX_Library.h>
#include "TouchDrvCSTXXX.hpp"
#include <Wire.h>

#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

#define I2C_SDA 8
#define I2C_SCL 7
#define TOUCH_IRQ 11


#define GFX_BL LCD_BL

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0 /* rotation */, true /* IPS */,
  240 /* width */, 280 /* height */,
  0 /* col offset 1 */, 20 /* row offset 1 */,
  0 /* col offset 2 */, 20 /* row offset 2 */);

TouchDrvCSTXXX touch;
bool isPressed = false;

void setup(void) {
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif
  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

  // Init Touch
  bool result = touch.begin(Wire, CST816_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
  if (result == false) {
    while (1) {
      Serial.println("Failed to initialize CST series touch, please check the connection...");
      delay(1000);
    }
  }

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  attachInterrupt(
    TOUCH_IRQ, []() {
      isPressed = true;
    },
    FALLING);

  // gfx->setTextColor(RGB565_WHITE);
  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
  gfx->setCursor(80, 140);
  gfx->printf("X:  0 Y:  0");
}

void loop() {
  int16_t x[5], y[5];
  if (isPressed) {
    isPressed = false;
    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
    if (touched) {
      gfx->drawCircle(x[0], y[0], 2, RED);
      gfx->setCursor(80, 140);
      gfx->printf("X:%3d Y:%3d", x[0], y[0]);
    }
  }
  delay(5);
}
