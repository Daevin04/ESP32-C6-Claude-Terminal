/*******************************************************************************
 * ESP32-C6 Serial Terminal with Touch Scrolling
 *
 * This creates a terminal interface that:
 * - Receives messages via Serial and displays them
 * - Sends messages via Serial using buttons/touch
 * - Supports touch scrolling up/down
 * - Works with ESP32-C6 hardware buttons
 *
 * Based on LVGL_Arduino_v9 example
 *
 * Requirements:
 * - Arduino_GFX_Library
 * - LVGL library
 * - Touch panel configured in touch.h
 ******************************************************************************/

#include <lvgl.h>
#include "TouchDrvCSTXXX.hpp"
#include <Wire.h>

/*******************************************************************************
 * ESP32-C6 Pin Configuration - Matching your working HelloWorld example
 ******************************************************************************/
// === LCD Pins === (from your working example)
#define LCD_SCK   1
#define LCD_DIN   2
#define LCD_CS    5
#define LCD_DC    3
#define LCD_RST   4
#define LCD_BL    6

// === Touch Pins === (from your working example)
#define I2C_SDA   8
#define I2C_SCL   7
#define TOUCH_IRQ 11

// Hardware buttons for ESP32-C6 (matching your working example)
#define BUTTON_UP_PIN    7   // BTN1 from your example
#define BUTTON_DOWN_PIN  8   // BTN2 from your example
#define BUTTON_SEND_PIN  9   // BTN3 from your example

/*******************************************************************************
 * Arduino_GFX setting - Matching your working HelloWorld configuration
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

#define GFX_BL LCD_BL

// Use exact same configuration as your working HelloWorld example
Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0 /* rotation */, true /* IPS */,
  240 /* width */, 280 /* height */,
  0 /* col offset 1 */, 20 /* row offset 1 */,
  0 /* col offset 2 */, 20 /* row offset 2 */);

/*******************************************************************************
 * Touch panel configuration - Using same system as your working example
 ******************************************************************************/
TouchDrvCSTXXX touch;
volatile bool isPressed = false;
#define CST816_SLAVE_ADDRESS 0x15

void IRAM_ATTR touchISR() {
  isPressed = true;
}

/*******************************************************************************
 * LVGL Display and UI Variables
 ******************************************************************************/
uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_display_t *disp;
lv_color_t *disp_draw_buf;

// UI Elements
lv_obj_t *main_screen;
lv_obj_t *terminal_area;
lv_obj_t *input_area;
lv_obj_t *send_btn;
lv_obj_t *scroll_up_btn;
lv_obj_t *scroll_down_btn;
lv_obj_t *status_label;

// Terminal buffer
String terminal_buffer = "";
String input_buffer = "";
const int MAX_TERMINAL_LINES = 100;
int terminal_lines = 0;

// Button states for debouncing
bool last_up_state = HIGH;
bool last_down_state = HIGH;
bool last_send_state = HIGH;
unsigned long last_button_time = 0;
const unsigned long BUTTON_DEBOUNCE = 200;

// Test message feature for scrolling demonstration
unsigned long last_test_message = 0;
const unsigned long TEST_MESSAGE_INTERVAL = 2000; // Send test message every 2 seconds
int test_message_counter = 1;
bool enable_test_messages = true; // Set to false to disable test messages
const int MAX_TEST_MESSAGES = 5; // Stop auto tests after 5 messages

/*******************************************************************************
 * LVGL Callback Functions
 ******************************************************************************/
#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

uint32_t millis_cb(void)
{
  return millis();
}

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
#ifndef DIRECT_RENDER_MODE
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
#endif
  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  static bool was_pressed = false;
  static int16_t last_x = 0, last_y = 0;

  if (isPressed) {
    isPressed = false;
    int16_t x[5], y[5];
    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());

    if (touched) {
      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = x[0];
      data->point.y = y[0];
      last_x = x[0];
      last_y = y[0];
      was_pressed = true;

      // Handle scrolling like in your working example
      if (y[0] < 140) {
        lv_obj_scroll_by(terminal_area, 0, 50, LV_ANIM_ON);
      } else if (y[0] >= 140) {
        lv_obj_scroll_by(terminal_area, 0, -50, LV_ANIM_ON);
      }
    }
  } else if (was_pressed) {
    data->state = LV_INDEV_STATE_RELEASED;
    data->point.x = last_x;
    data->point.y = last_y;
    was_pressed = false;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

/*******************************************************************************
 * Terminal Functions
 ******************************************************************************/
void add_terminal_text(const String &text)
{
  // Add timestamp
  String timestamp = "[" + String(millis() / 1000) + "s] ";
  String full_text = timestamp + text + "\n";

  terminal_buffer += full_text;
  terminal_lines++;

  // Limit buffer size
  if (terminal_lines > MAX_TERMINAL_LINES)
  {
    int first_newline = terminal_buffer.indexOf('\n');
    if (first_newline != -1)
    {
      terminal_buffer = terminal_buffer.substring(first_newline + 1);
      terminal_lines--;
    }
  }

  // Update the text area
  lv_textarea_set_text(terminal_area, terminal_buffer.c_str());

  // Auto-scroll to bottom
  lv_obj_scroll_to_y(terminal_area, LV_COORD_MAX, LV_ANIM_ON);
}

void send_serial_message(const String &message)
{
  Serial.println(message);
  add_terminal_text("SENT: " + message);

  // Update status
  lv_label_set_text(status_label, ("Last sent: " + message).c_str());
}

/*******************************************************************************
 * Button Event Handlers
 ******************************************************************************/
static void send_btn_event_handler(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED)
  {
    const char* text = lv_textarea_get_text(input_area);
    String message = String(text);

    if(message.length() > 0)
    {
      send_serial_message(message);
      lv_textarea_set_text(input_area, ""); // Clear input
    }
  }
}

static void scroll_up_btn_event_handler(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED)
  {
    lv_obj_scroll_by(terminal_area, 0, 50, LV_ANIM_ON);
    send_serial_message("UP");
  }
}

static void scroll_down_btn_event_handler(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED)
  {
    lv_obj_scroll_by(terminal_area, 0, -50, LV_ANIM_ON);
    send_serial_message("DOWN");
  }
}

/*******************************************************************************
 * UI Creation Functions
 ******************************************************************************/
void create_ui()
{
  main_screen = lv_scr_act();

  // Status label at top
  status_label = lv_label_create(main_screen);
  lv_label_set_text(status_label, "ESP32-C6 Serial Terminal Ready");
  lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 5);
  lv_obj_set_style_text_color(status_label, lv_color_hex(0x00FF00), 0);

  // Terminal text area (scrollable)
  terminal_area = lv_textarea_create(main_screen);
  lv_obj_set_size(terminal_area, screenWidth - 20, screenHeight * 0.6);
  lv_obj_align(terminal_area, LV_ALIGN_TOP_MID, 0, 30);
  lv_textarea_set_text(terminal_area, "=== ESP32-C6 Serial Terminal ===\nWaiting for messages...\n");
  lv_obj_set_style_bg_color(terminal_area, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_color(terminal_area, lv_color_hex(0x00FF00), 0);
  lv_obj_clear_flag(terminal_area, LV_OBJ_FLAG_CLICKABLE);

  // Input area
  input_area = lv_textarea_create(main_screen);
  lv_obj_set_size(input_area, screenWidth * 0.7, 40);
  lv_obj_align(input_area, LV_ALIGN_BOTTOM_LEFT, 10, -50);
  lv_textarea_set_placeholder_text(input_area, "Type message...");
  lv_textarea_set_one_line(input_area, true);

  // Send button
  send_btn = lv_btn_create(main_screen);
  lv_obj_set_size(send_btn, screenWidth * 0.25, 40);
  lv_obj_align(send_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -50);
  lv_obj_add_event_cb(send_btn, send_btn_event_handler, LV_EVENT_CLICKED, NULL);

  lv_obj_t *send_label = lv_label_create(send_btn);
  lv_label_set_text(send_label, "SEND");
  lv_obj_center(send_label);

  // Scroll buttons
  scroll_up_btn = lv_btn_create(main_screen);
  lv_obj_set_size(scroll_up_btn, 60, 40);
  lv_obj_align(scroll_up_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
  lv_obj_add_event_cb(scroll_up_btn, scroll_up_btn_event_handler, LV_EVENT_CLICKED, NULL);

  lv_obj_t *up_label = lv_label_create(scroll_up_btn);
  lv_label_set_text(up_label, "UP");
  lv_obj_center(up_label);

  scroll_down_btn = lv_btn_create(main_screen);
  lv_obj_set_size(scroll_down_btn, 60, 40);
  lv_obj_align(scroll_down_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_add_event_cb(scroll_down_btn, scroll_down_btn_event_handler, LV_EVENT_CLICKED, NULL);

  lv_obj_t *down_label = lv_label_create(scroll_down_btn);
  lv_label_set_text(down_label, "DOWN");
  lv_obj_center(down_label);
}

/*******************************************************************************
 * Hardware Button Handling
 ******************************************************************************/
void handle_hardware_buttons()
{
  unsigned long current_time = millis();
  if (current_time - last_button_time < BUTTON_DEBOUNCE) return;

  bool up_pressed = !digitalRead(BUTTON_UP_PIN);
  bool down_pressed = !digitalRead(BUTTON_DOWN_PIN);
  bool send_pressed = !digitalRead(BUTTON_SEND_PIN);

  // Up button
  if (up_pressed && last_up_state != up_pressed)
  {
    lv_obj_scroll_by(terminal_area, 0, 50, LV_ANIM_ON);
    send_serial_message("BUTTON_UP");
    last_button_time = current_time;
  }

  // Down button
  if (down_pressed && last_down_state != down_pressed)
  {
    lv_obj_scroll_by(terminal_area, 0, -50, LV_ANIM_ON);
    send_serial_message("BUTTON_DOWN");
    last_button_time = current_time;
  }

  // Send button
  if (send_pressed && last_send_state != send_pressed)
  {
    const char* text = lv_textarea_get_text(input_area);
    String message = String(text);

    if(message.length() > 0)
    {
      send_serial_message(message);
      lv_textarea_set_text(input_area, "");
    }
    else
    {
      send_serial_message("BUTTON_SEND");
    }
    last_button_time = current_time;
  }

  last_up_state = up_pressed;
  last_down_state = down_pressed;
  last_send_state = send_pressed;
}

/*******************************************************************************
 * Test Message Generation (for scroll testing)
 ******************************************************************************/
void handle_test_messages()
{
  if (!enable_test_messages) return;

  // Stop after MAX_TEST_MESSAGES
  if (test_message_counter > MAX_TEST_MESSAGES) {
    enable_test_messages = false;
    add_terminal_text("SYSTEM: Auto test completed (5 messages sent)");
    lv_label_set_text(status_label, "Ready for Raspberry Pi communication");
    return;
  }

  unsigned long current_time = millis();
  if (current_time - last_test_message >= TEST_MESSAGE_INTERVAL)
  {
    // Generate different types of test messages
    String test_messages[] = {
      "Test message #" + String(test_message_counter) + " - This is a short message",
      "Test #" + String(test_message_counter) + " - This is a much longer test message to demonstrate text wrapping and scrolling behavior in the terminal",
      "MSG " + String(test_message_counter) + " - Testing scrolling with various message lengths",
      "Demo #" + String(test_message_counter) + " - Raspberry Pi communication simulation: sensor data, status updates, commands",
      "Status " + String(test_message_counter) + " - System running normally, all sensors OK, temperature: 25.3°C",
      "Alert #" + String(test_message_counter) + " - This is a longer alert message that spans multiple lines to test the scrolling",
      "Data " + String(test_message_counter) + " - JSON: {\"temp\":23.5,\"humidity\":65,\"status\":\"ok\"}",
      "Log #" + String(test_message_counter) + " - Simulating various log levels and message types"
    };

    int message_index = (test_message_counter - 1) % 8;
    String test_msg = test_messages[message_index];

    add_terminal_text("TEST: " + test_msg);

    // Update status to show test progress
    lv_label_set_text(status_label, ("Auto-test: " + String(test_message_counter) + "/" + String(MAX_TEST_MESSAGES)).c_str());

    test_message_counter++;
    last_test_message = current_time;
  }
}

/*******************************************************************************
 * Serial Communication Handling
 ******************************************************************************/
void handle_serial_communication()
{
  while (Serial.available())
  {
    String received_message = Serial.readStringUntil('\n');
    received_message.trim();

    if (received_message.length() > 0)
    {
      // Check for special commands to control test messages
      if (received_message == "STOP_TEST")
      {
        enable_test_messages = false;
        add_terminal_text("SYSTEM: Test messages stopped");
        lv_label_set_text(status_label, "Test messages disabled");
        return;
      }
      else if (received_message == "START_TEST")
      {
        enable_test_messages = true;
        test_message_counter = 1;
        add_terminal_text("SYSTEM: Test messages started");
        lv_label_set_text(status_label, "Test messages enabled");
        return;
      }

      add_terminal_text("RX: " + received_message);

      // Update status
      lv_label_set_text(status_label, ("Received: " + received_message).c_str());
    }

  }
}

/*******************************************************************************
 * Setup Function
 ******************************************************************************/
void setup()
{
  // Initialize I2C and Serial (like your working example)
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  Serial.println("ESP32-C6 Serial Terminal Starting...");

  // Initialize Display (exact same as your working example)
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

  // Initialize Touch (exact same as your working example)
  if (!touch.begin(Wire, CST816_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println("Touch init failed!");
    while (1) delay(1000);
  }

  // Initialize backlight (exact same as your working example)
#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  // Initialize touch interrupt (exact same as your working example)
  pinMode(TOUCH_IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOUCH_IRQ), touchISR, FALLING);

  // Initialize hardware buttons
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SEND_PIN, INPUT_PULLUP);

  // Initialize LVGL
  lv_init();
  lv_tick_set_cb(millis_cb);

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  screenWidth = gfx->width();
  screenHeight = gfx->height();

#ifdef DIRECT_RENDER_MODE
  bufSize = screenWidth * screenHeight;
#else
  bufSize = screenWidth * 40;
#endif

  // Allocate display buffer
#ifdef ESP32
#if defined(DIRECT_RENDER_MODE) && (defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL))
  disp_draw_buf = (lv_color_t *)gfx->getFramebuffer();
#else
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_draw_buf)
  {
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
  }
#endif
#else
  disp_draw_buf = (lv_color_t *)malloc(bufSize * 2);
#endif

  if (!disp_draw_buf)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
    while(1);
  }

  // Create display
  disp = lv_display_create(screenWidth, screenHeight);
  lv_display_set_flush_cb(disp, my_disp_flush);

#ifdef DIRECT_RENDER_MODE
  lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize * 2, LV_DISPLAY_RENDER_MODE_DIRECT);
#else
  lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

  // Initialize input device (touchscreen)
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  // Create UI
  create_ui();

  Serial.println("ESP32-C6 Serial Terminal Ready!");
  Serial.println("Commands: Send any text to display it");
  Serial.println("Touch screen to scroll, use buttons for quick commands");
  Serial.println("Special commands: STOP_TEST (disable test), START_TEST (enable test)");

  add_terminal_text("Terminal initialized successfully!");
  add_terminal_text("Ready to receive messages from Raspberry Pi 5");
  add_terminal_text("SYSTEM: Auto test messages enabled (every 2s)");
  add_terminal_text("SYSTEM: Send 'STOP_TEST' to disable test messages");
}

/*******************************************************************************
 * Main Loop
 ******************************************************************************/
void loop()
{
  // Handle LVGL tasks
  lv_timer_handler();

  // Handle test messages (for scroll testing)
  handle_test_messages();

  // Handle incoming serial messages
  handle_serial_communication();

  // Handle hardware buttons
  handle_hardware_buttons();

#ifdef DIRECT_RENDER_MODE
#if defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL)
  gfx->flush();
#else
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#endif
#else
#ifdef CANVAS
  gfx->flush();
#endif
#endif

  delay(5);
}