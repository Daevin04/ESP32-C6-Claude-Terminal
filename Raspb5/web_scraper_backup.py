import time, serial
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service

# === Serial setup ===
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)

# === Start Chromium ===
options = webdriver.ChromeOptions()
options.add_argument("--headless=new")   # run without GUI
options.add_argument("--no-sandbox")
options.add_argument("--disable-dev-shm-usage")

# Use system ChromeDriver for ARM64
driver = webdriver.Chrome(service=Service("/usr/bin/chromedriver"), options=options)

# === Open a random site (example: Wikipedia) ===
driver.get("https://en.wikipedia.org/wiki/Raspberry_Pi")

# Grab the first paragraph text
para = driver.find_element(By.CSS_SELECTOR, "p").text.strip()

print("Extracted text:\n", para)

# Save to file
with open("output.txt", "w", encoding="utf-8") as f:
    f.write(para)

# Send to ESP32-C6 over serial
ser.write((para + "\n").encode("utf-8"))

print("Sent to ESP32-C6!")

driver.quit()