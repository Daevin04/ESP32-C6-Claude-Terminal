#include <Arduino_GFX_Library.h>
#include "image.h"
#define LCD_SCK   1
#define LCD_DIN   2
#define LCD_CS    5
#define LCD_DC    3
#define LCD_RST   4
#define LCD_BL    6

#define GFX_BL LCD_BL

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0 /* rotation */, true /* IPS */,
  240 /* width */, 280 /* height */,
  0 /* col offset 1 */, 20 /* row offset 1 */,
  0 /* col offset 2 */, 20 /* row offset 2 */);

void setup(void)
{
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)gImage_image, 240, 280);
}

void loop()
{
  delay(1000); // 1 second
}
