
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>

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
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/* more fonts at: https://github.com/moononournation/ArduinoFreeFontFile.git */

void setup(void)
{
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX U8g2 Font Print UTF8 example");

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);
  gfx->setUTF8Print(true); // enable UTF8 support for the Arduino print() function

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  int16_t x1, y1;
  uint16_t w, h;

  /* U8g2 font list: https://github.com/olikraus/u8g2/wiki/fntlistall */
  /* U8g2 Unifont list: https://github.com/olikraus/u8g2/wiki/fntgrpunifont */
  gfx->setFont(u8g2_font_unifont_tr);
  gfx->setTextColor(RGB565_RED);
  gfx->setCursor(20, 16);
  // gfx->getTextBounds("Hello World!", 1, 16, &x1, &y1, &w, &h);
  // gfx->drawRect(x1 - 1, y1 - 1, w + 2, h + 2, RGB565_RED);
  gfx->println("Hello World!");

  gfx->setFont(u8g2_font_unifont_t_polish);
  gfx->setTextColor(RGB565_YELLOW);
  gfx->setCursor(20, 36);
  // gfx->getTextBounds("Witaj świecie!", 1, 36, &x1, &y1, &w, &h);
  // gfx->drawRect(x1 - 1, y1 - 1, w + 2, h + 2, RGB565_RED);
  gfx->println("Witaj świecie!");

  gfx->setFont(u8g2_font_unifont_t_vietnamese1);
  gfx->setTextColor(RGB565_GREEN);
  gfx->setCursor(20, 56);
  // gfx->getTextBounds("Chào thế giới!", 1, 56, &x1, &y1, &w, &h);
  // gfx->drawRect(x1 - 1, y1 - 1, w + 2, h + 2, RGB565_RED);
  gfx->println("Chào thế giới!");

#ifdef U8G2_USE_LARGE_FONTS
  gfx->setFont(u8g2_font_unifont_t_chinese2);
  gfx->setTextColor(RGB565_CYAN);
  gfx->setCursor(20, 76);
  // gfx->getTextBounds("世界你好!", 1, 76, &x1, &y1, &w, &h);
  // gfx->drawRect(x1 - 1, y1 - 1, w + 2, h + 2, RGB565_RED);
  gfx->println("世界你好!");

  gfx->setFont(u8g2_font_unifont_t_japanese1);
  gfx->setTextColor(RGB565_BLUE);
  gfx->setCursor(20, 96);
  // gfx->getTextBounds("こんにちは世界!", 1, 96, &x1, &y1, &w, &h);
  // gfx->drawRect(x1 - 1, y1 - 1, w + 2, h + 2, RGB565_RED);
  gfx->println("こんにちは世界!");

  gfx->setFont(u8g2_font_unifont_t_korean1);
  gfx->setTextColor(RGB565_MAGENTA);
  gfx->setCursor(20, 116);
  // gfx->getTextBounds("안녕하세요, 세계입니다!", 1, 116, &x1, &y1, &w, &h);
  // gfx->drawRect(x1 - 1, y1 - 1, w + 2, h + 2, RGB565_RED);
  gfx->println("안녕하세요, 세계입니다!");
#endif // U8G2_USE_LARGE_FONTS
}

void loop()
{
}