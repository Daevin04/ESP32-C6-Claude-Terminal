/*******************************************************************************
 * ESP32-C6 Touch Terminal with Scrolling - CLEAN VERSION
 * Created: After removing timestamps and disabling all test messages
 *
 * This creates a terminal interface that:
 * - Receives messages via Serial and displays them (without timestamps)
 * - PRIMARY NAVIGATION: Touch screen (Top half=UP, Bottom half=DOWN)
 * - SECONDARY NAVIGATION: Physical UP/DOWN buttons
 * - No text input - focused on touch-based scrolling
 * - Works with ESP32-C6 hardware buttons
 * - All test functions disabled for clean Pi 5 communication
 *
 * Based on LVGL_Arduino_v9 example
 *
 * Requirements:
 * - Arduino_GFX_Library
 * - LVGL library
 * - Touch panel configured (CST816S)
 ******************************************************************************/

#include <lvgl.h>
#include "TouchDrvCSTXXX.hpp"
#include <Wire.h>
#include "OneButton.h"

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
#define BUTTON_DOWN_PIN  18   // BTN2 from your example
#define BUTTON_SEND_PIN  9   // BTN3 from your example - physical send button

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
// Touch setup - EXACT copy from working 15_touch_example.ino
TouchDrvCSTXXX touch;
bool isPressed = false;

// Define CST816_SLAVE_ADDRESS (same as in library)
#define CST816_SLAVE_ADDRESS 0x15

// OneButton setup for BUTTON_DOWN_PIN (GPIO 18) - double-click functionality
OneButton downButton(BUTTON_DOWN_PIN, true);

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
lv_obj_t *status_label;

// Terminal buffer
String terminal_buffer = "";
String input_buffer = "";
const int MAX_BUFFER_SIZE = 30000;  // Character-based limit instead of line-based
// Auto-scroll completely disabled - all scrolling is manual only

// Button states for debouncing (BUTTON_DOWN_PIN now handled by OneButton)
bool last_up_state = HIGH;
bool last_send_state = HIGH;
unsigned long last_button_time = 0;
const unsigned long BUTTON_DEBOUNCE = 200;

// Test message feature for scrolling demonstration
unsigned long last_test_message = 0;
const unsigned long TEST_MESSAGE_INTERVAL = 2000; // Send test message every 2 seconds
int test_message_counter = 1;
bool enable_test_messages = false; // Set to false to disable test messages
const int MAX_TEST_MESSAGES = 5; // Stop auto tests after 5 messages

// Scroll test loop variables
unsigned long last_scroll_test = 0;
const unsigned long SCROLL_TEST_INTERVAL = 3000; // Test scroll every 3 seconds
bool scroll_test_up = true; // true = scroll up, false = scroll down
bool enable_scroll_test = false; // Set to false to disable scroll test

// Pi 5 sender test variables
unsigned long last_pi_message = 0;
const unsigned long PI_MESSAGE_INTERVAL = 3000; // Send message to Pi every 3 seconds
int pi_message_counter = 1;
bool enable_pi_sender = false; // Set to false to disable Pi sender test

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

// Disable LVGL touch handling - we'll use direct touch handling like 15_touch_example
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  // Always report as released - we handle touch directly
  data->state = LV_INDEV_STATE_RELEASED;
}

/*******************************************************************************
 * Terminal Functions
 ******************************************************************************/
void add_terminal_text_chunked(const String &text)
{
  const int MAX_CHUNK_SIZE = 200;  // Based on your testing - max ~254 chars, using 200 for safety

  // If text is short enough, add it normally
  if (text.length() <= MAX_CHUNK_SIZE)
  {
    String full_text = text + "\n";

    // Check buffer size and clear if needed
    if (terminal_buffer.length() + full_text.length() > MAX_BUFFER_SIZE)
    {
      int keep_from = terminal_buffer.length() - 15000;
      if (keep_from > 0)
      {
        int newline_pos = terminal_buffer.indexOf('\n', keep_from);
        if (newline_pos != -1)
        {
          terminal_buffer = "...[OLDER CONTENT CLEARED]...\n" + terminal_buffer.substring(newline_pos + 1);
        }
        else
        {
          terminal_buffer = "...[BUFFER CLEARED]...\n";
        }
      }
    }

    terminal_buffer += full_text;
    lv_textarea_set_text(terminal_area, terminal_buffer.c_str());
    return;
  }

  // Long text - split into chunks
  int chunk_num = 1;
  int total_chunks = (text.length() + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;  // Round up division

  for (int i = 0; i < text.length(); i += MAX_CHUNK_SIZE)
  {
    String chunk = text.substring(i, i + MAX_CHUNK_SIZE);

    // Add chunk indicators
    String chunk_header = "[PART " + String(chunk_num) + "/" + String(total_chunks) + "]\n";
    String full_chunk = chunk_header + chunk + "\n";

    // Check buffer size and clear if needed
    if (terminal_buffer.length() + full_chunk.length() > MAX_BUFFER_SIZE)
    {
      int keep_from = terminal_buffer.length() - 15000;
      if (keep_from > 0)
      {
        int newline_pos = terminal_buffer.indexOf('\n', keep_from);
        if (newline_pos != -1)
        {
          terminal_buffer = "...[OLDER CONTENT CLEARED]...\n" + terminal_buffer.substring(newline_pos + 1);
        }
        else
        {
          terminal_buffer = "...[BUFFER CLEARED]...\n";
        }
      }
    }

    // Add chunk to buffer
    terminal_buffer += full_chunk;
    lv_textarea_set_text(terminal_area, terminal_buffer.c_str());

    chunk_num++;

    // Small delay between chunks for readability
    delay(100);
  }

  // No completion marker needed - part numbers show progress
}

void add_terminal_text(const String &text)
{
  add_terminal_text_chunked(text);
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
// On-screen button event handlers removed - using hardware buttons only

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

  // Terminal text area (scrollable) - Full screen minus status area
  terminal_area = lv_textarea_create(main_screen);
  lv_obj_set_size(terminal_area, screenWidth - 20, screenHeight - 50);
  lv_obj_align(terminal_area, LV_ALIGN_TOP_MID, 0, 30);
  lv_textarea_set_text(terminal_area, "=== ESP32-C6 Serial Terminal ===\nWaiting for messages...\n");
  lv_obj_set_style_bg_color(terminal_area, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_color(terminal_area, lv_color_hex(0x00FF00), 0);
  lv_obj_set_style_text_font(terminal_area, &lv_font_montserrat_12, 0);  // Smaller text size
  lv_obj_clear_flag(terminal_area, LV_OBJ_FLAG_CLICKABLE);

  // Set large buffer size for long GPT responses
  lv_textarea_set_max_length(terminal_area, 32768);  // 32KB text buffer

  // Enable scrolling and make sure scrollbars are visible
  lv_obj_set_scrollbar_mode(terminal_area, LV_SCROLLBAR_MODE_AUTO);
  lv_obj_add_flag(terminal_area, LV_OBJ_FLAG_SCROLLABLE);

  // All on-screen controls removed - using hardware buttons and touch only
}

/*******************************************************************************
 * OneButton Callback Functions for BUTTON_DOWN_PIN
 ******************************************************************************/
// Single click on DOWN button - scroll up
void downButtonSingleClick() {
  lv_obj_scroll_by(terminal_area, 0, 30, LV_ANIM_ON);  // Positive value scrolls up
}

// Double click on DOWN button - send serial message
void downButtonDoubleClick() {
  Serial.println("ImageUpload");  // Send this message to Pi 5
  add_terminal_text("TX: ImageUpload");
  lv_label_set_text(status_label, "Sent: ImageUpload");
}

/*******************************************************************************
 * Hardware Button Handling
 ******************************************************************************/
void handle_hardware_buttons()
{
  unsigned long current_time = millis();
  if (current_time - last_button_time < BUTTON_DEBOUNCE) return;

  bool up_pressed = !digitalRead(BUTTON_UP_PIN);
  bool send_pressed = !digitalRead(BUTTON_SEND_PIN);

  // BUTTON_UP_PIN (GPIO 7) - scroll up in terminal
  if (up_pressed && last_up_state != up_pressed)
  {
    lv_obj_scroll_by(terminal_area, 0, 30, LV_ANIM_ON);
    last_button_time = current_time;
  }

  // BUTTON_DOWN_PIN (GPIO 18) - handled by OneButton library
  // Single click: scroll down
  // Double click: send ImageUpload command
  downButton.tick();  // Process OneButton events

  // BUTTON_SEND_PIN (GPIO 9) - scroll down like DOWN button
  if (send_pressed && last_send_state != send_pressed)
  {
    lv_obj_scroll_by(terminal_area, 0, -60, LV_ANIM_ON);
    last_button_time = current_time;
  }

  last_up_state = up_pressed;
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
 * Touch Handler - EXACT copy from working 15_touch_example.ino loop()
 ******************************************************************************/
//void handle_working_touch()
//{
  // EXACT copy of the working loop() from 15_touch_example.ino
//  int16_t x[5], y[5];
//  if (isPressed) {
//    isPressed = false;
//    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
//    if (touched) {
      // Instead of drawing circle and printf, do scroll logic
//      Serial.printf("Touch at X:%d Y:%d\n", x[0], y[0]);

      // Y-based scrolling as requested
  //    if (y[0] < 145) {
        // Touch top half - scroll UP (show older messages)
    //    add_terminal_text("TOUCH UP at Y=" + String(y[0]));
        // Manual scrolling only
        // Scroll up using scroll_by with negative value
//        lv_obj_scroll_by(terminal_area, 0, -30, LV_ANIM_ON);
  //      Serial.println("Touch UP: scrolling by -20 pixels");
    //  } else {
        // Touch bottom half - scroll DOWN (show newer messages)
  //      add_terminal_text("TOUCH DOWN at Y=" + String(y[0]));
        // Manual scrolling only
        // Scroll down using scroll_by with positive value
    //    lv_obj_scroll_by(terminal_area, 0, 30, LV_ANIM_ON);
     //   Serial.println("Touch DOWN: scrolling by +20 pixels");
  //    }
  //  }
 // }
//}

/*******************************************************************************
 * Scroll Test Loop - Automatically test scrolling up and down
 ******************************************************************************/
void handle_scroll_test()
{
  if (!enable_scroll_test) return;

  unsigned long current_time = millis();
  if (current_time - last_scroll_test >= SCROLL_TEST_INTERVAL)
  {
    // Get current scrollbar position and boundaries
    int32_t current_y = lv_obj_get_scroll_y(terminal_area);
    int32_t scroll_top = lv_obj_get_scroll_top(terminal_area);
    int32_t scroll_bottom = lv_obj_get_scroll_bottom(terminal_area);
    int32_t total_scrollable = scroll_top + scroll_bottom;

    Serial.println("=== SCROLL POSITION INFO ===");
    Serial.println("Current Y: " + String(current_y));
    Serial.println("Scroll Top: " + String(scroll_top));
    Serial.println("Scroll Bottom: " + String(scroll_bottom));
    Serial.println("Total Scrollable: " + String(total_scrollable));

    if (scroll_test_up)
    {
      // Test scroll to 25% position
      int32_t target_y = total_scrollable * 0.25f;
      lv_obj_scroll_to_y(terminal_area, target_y, LV_ANIM_ON);
      add_terminal_text("SCROLL TEST: Moving to 25% position (Y=" + String(target_y) + ")");
      Serial.println("SCROLL TEST: UP - scrolling to 25% position Y=" + String(target_y));
      scroll_test_up = false; // Next time scroll down
    }
    else
    {
      // Test scroll to 75% position
      int32_t target_y = total_scrollable * 0.75f;
      lv_obj_scroll_to_y(terminal_area, target_y, LV_ANIM_ON);
      add_terminal_text("SCROLL TEST: Moving to 75% position (Y=" + String(target_y) + ")");
      Serial.println("SCROLL TEST: DOWN - scrolling to 75% position Y=" + String(target_y));
      scroll_test_up = true; // Next time scroll up
    }

    last_scroll_test = current_time;
  }
}


/*******************************************************************************
 * Pi 5 Message Sender Test - Continuously send messages to Raspberry Pi 5
 ******************************************************************************/
void handle_pi_sender_test()
{
  if (!enable_pi_sender) return;

  unsigned long current_time = millis();
  if (current_time - last_pi_message >= PI_MESSAGE_INTERVAL)
  {
    // Generate different types of test messages to send to Pi 5
    String pi_messages[] = {
      "ESP32_STATUS:OK",
      "SENSOR_TEMP:25.3",
      "SENSOR_HUMIDITY:65",
      "BUTTON_PRESSED:UP",
      "SYSTEM_MEMORY:45%",
      "WIFI_SIGNAL:-65dBm",
      "BATTERY:78%",
      "GPS_LAT:40.7128,GPS_LON:-74.0060"
    };

    int message_index = (pi_message_counter - 1) % 8;
    String message_to_send = pi_messages[message_index] + "_#" + String(pi_message_counter);

    // Send message to Pi 5 via Serial
    Serial.println(message_to_send);

    // Also display in terminal what we sent
    add_terminal_text("TX: " + message_to_send);

    // Update status
    lv_label_set_text(status_label, ("Sent to Pi: " + message_to_send).c_str());

    pi_message_counter++;
    last_pi_message = current_time;

    Serial.println("DEBUG: Sent message #" + String(pi_message_counter-1) + " to Pi 5");
  }
}

/*******************************************************************************
 * Serial Communication Handling
 ******************************************************************************/
void handle_serial_communication()
{
  // Simple approach: read all available data at once
  if (Serial.available())
  {
    String received_message = Serial.readString();  // Read ALL available data
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
      else if (received_message == "STOP_SCROLL_TEST")
      {
        enable_scroll_test = false;
        add_terminal_text("SYSTEM: Scroll test stopped");
        lv_label_set_text(status_label, "Scroll test disabled");
        return;
      }
      else if (received_message == "START_SCROLL_TEST")
      {
        enable_scroll_test = true;
        add_terminal_text("SYSTEM: Scroll test started");
        lv_label_set_text(status_label, "Scroll test enabled");
        return;
      }
      else if (received_message == "STOP_PI_SENDER")
      {
        enable_pi_sender = false;
        add_terminal_text("SYSTEM: Pi sender test stopped");
        lv_label_set_text(status_label, "Pi sender disabled");
        return;
      }
      else if (received_message == "START_PI_SENDER")
      {
        enable_pi_sender = true;
        pi_message_counter = 1;
        add_terminal_text("SYSTEM: Pi sender test started");
        lv_label_set_text(status_label, "Pi sender enabled");
        return;
      }

      add_terminal_text("RX: " + received_message);

      // Update status
      lv_label_set_text(status_label, "Received GPT response");
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

  // Init Touch - EXACT copy from working 15_touch_example.ino
  bool result = touch.begin(Wire, CST816_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
  if (result == false) {
    while (1) {
      Serial.println("Failed to initialize CST series touch, please check the connection...");
      delay(1000);
    }
  }

  // Initialize backlight (exact same as your working example)
#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  // Touch interrupt - EXACT copy from working 15_touch_example.ino
  attachInterrupt(
    TOUCH_IRQ, []() {
      isPressed = true;
    },
    FALLING);

  // Initialize hardware buttons
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SEND_PIN, INPUT_PULLUP);

  // Setup OneButton for BUTTON_DOWN_PIN (GPIO 18)
  // pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP); // OneButton handles this
  downButton.attachClick(downButtonSingleClick);
  downButton.attachDoubleClick(downButtonDoubleClick);

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

  // Startup message removed to avoid interference with Pi 5 communication

  add_terminal_text("Touch Terminal initialized successfully!");
  add_terminal_text("Touch top half (Y<145) for UP, bottom half for DOWN");
  add_terminal_text("Physical UP/DOWN buttons also available");
  add_terminal_text("Ready to receive messages from Raspberry Pi 5");
  add_terminal_text("SYSTEM: Auto test messages enabled (every 2s)");
  add_terminal_text("SYSTEM: Auto scroll test enabled (every 3s)");
  add_terminal_text("Commands: STOP_TEST, START_TEST, STOP_SCROLL_TEST, START_SCROLL_TEST");
}

/*******************************************************************************
 * Main Loop
 ******************************************************************************/
void loop()
{
  // Handle LVGL tasks
  lv_timer_handler();

  // Handle test messages (DISABLED)
  // handle_test_messages();

  // Handle scroll test loop (DISABLED)
  // handle_scroll_test();

  // Handle Pi 5 sender test (DISABLED)
  // handle_pi_sender_test();

  // Handle incoming serial messages
  handle_serial_communication();

  // Handle hardware buttons
  handle_hardware_buttons();

  // Handle touch - DISABLED (function is commented out)
  // handle_working_touch();

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