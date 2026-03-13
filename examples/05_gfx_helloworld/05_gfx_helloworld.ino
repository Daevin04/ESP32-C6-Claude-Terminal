#include <Arduino_GFX_Library.h>
#include "TouchDrvCSTXXX.hpp"
#include <Wire.h>

// === LCD Pins ===
#define LCD_SCK   1
#define LCD_DIN   2
#define LCD_CS    5
#define LCD_DC    3
#define LCD_RST   4
#define LCD_BL    6

// === Touch Pins ===
#define I2C_SDA   8
#define I2C_SCL   7
#define TOUCH_IRQ 11
#define CST816_SLAVE_ADDRESS 0x15

// === Buttons for Pi actions ===
#define BTN1 7   // photo
#define BTN2 8   // get new
#define BTN3 9   // send instr

#define GFX_BL LCD_BL

// --- Display setup ---
Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0 /* rotation */, true /* IPS */,
  240 /* width */, 280 /* height */,
  0 /* col offset 1 */, 20 /* row offset 1 */,
  0 /* col offset 2 */, 20 /* row offset 2 */);

// --- Touch driver ---
TouchDrvCSTXXX touch;
volatile bool isPressed = false;

// --- Buffer for chat lines ---
#define MAX_LINES 100
#define LINE_HEIGHT 20
#define VISIBLE_LINES 12

String lines[MAX_LINES];
int lineCount = 0;
int scrollOffset = 0;  // 0 = bottom

// === Add line to buffer ===
void addLine(String msg) {
  if (lineCount < MAX_LINES) {
    lines[lineCount++] = msg;
  } else {
    for (int i = 1; i < MAX_LINES; i++) lines[i-1] = lines[i];
    lines[MAX_LINES-1] = msg;
  }
  scrollOffset = 0; // stick to bottom on new line
  redrawScreen();
}

// === Redraw window ===
void redrawScreen() {
  gfx->fillScreen(RGB565_BLACK);
  gfx->setTextColor(RGB565_WHITE);
  gfx->setTextSize(1);

  int start = lineCount - VISIBLE_LINES - scrollOffset;
  if (start < 0) start = 0;

  for (int i = 0; i < VISIBLE_LINES; i++) {
    int idx = start + i;
    if (idx < lineCount) {
      gfx->setCursor(5, 5 + i * LINE_HEIGHT);
      gfx->println(lines[idx]);
    }
  }
}

// === Example text ===
void showCustomMessage() {
  addLine("=== CUSTOM TEXT ===");
  addLine("a. process");
  addLine("b. architecture");
  addLine("c. entity and architecture");
  addLine("d. process");
  addLine("e. entity and architecture");
  addLine("f. delta delay");
  addLine("g. entity arch process");
  addLine("h. Event: sig changes");
  addLine("   after delta cycle");
  addLine("i. Transaction: driver");
  addLine("   updates signal now");
  addLine("=== CUSTOM TEXT ===");
  addLine("a. process");
  addLine("b. architecture");
  addLine("c. entity and architecture");
  addLine("d. process");
  addLine("e. entity and architecture");
  addLine("f. delta delay");
  addLine("g. entity arch process");
  addLine("h. Event: sig changes");
  addLine("   after delta cycle");
  addLine("i. Transaction: driver");
  addLine("   updates signal now");
}

void IRAM_ATTR touchISR() {
  isPressed = true;
}

void setup() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);

  // Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

  // Touch
  if (!touch.begin(Wire, CST816_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println("Touch init failed!");
    while (1) delay(1000);
  }

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  pinMode(TOUCH_IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOUCH_IRQ), touchISR, FALLING);

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);

  addLine("Ready. Swipe Up/Down to scroll");
  showCustomMessage();
}

void loop() {
  // Buttons → Pi actions
  if (digitalRead(BTN1) == LOW) {
    Serial.println("BTN1 → PHOTO");
    delay(300);
  }
  if (digitalRead(BTN2) == LOW) {
    Serial.println("BTN2 → GET NEW");
    delay(300);
  }
  if (digitalRead(BTN3) == LOW) {
    Serial.println("BTN3 → INSTR");
    delay(300);
  }

  // Touch → scroll
  if (isPressed) {
    isPressed = false;
    int16_t x[5], y[5];
    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
    if (touched) {
      // Simple swipe detection: top half = scroll up, bottom half = scroll down
      if (y[0] < 140 && scrollOffset < lineCount - VISIBLE_LINES) {
        scrollOffset++;
        redrawScreen();
      } else if (y[0] >= 140 && scrollOffset > 0) {
        scrollOffset--;
        redrawScreen();
      }
    }
  }

  // Serial → incoming lines
  while (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    if (msg.length() > 0) {
      addLine(msg);
    }
  }

  delay(5);
}
