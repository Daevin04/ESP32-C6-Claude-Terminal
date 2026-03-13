import time
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

# === Start Chromium ===
options = webdriver.ChromeOptions()
options.add_argument("--headless=new")   # run without GUI
options.add_argument("--no-sandbox")
options.add_argument("--disable-dev-shm-usage")

driver = webdriver.Chrome(service=Service("/usr/bin/chromedriver"), options=options)

try:
    # Go to ChatGPT
    driver.get("https://chatgpt.com/")

    # Wait and click Log in
    login_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//div[contains(text(), 'Log in')]"))
    )
    login_btn.click()

    time.sleep(2)

    # Click Continue with Google
    google_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//button[contains(text(), 'Continue with Google')]"))
    )
    google_btn.click()

    time.sleep(2)

    # Type email
    email_input = WebDriverWait(driver, 10).until(
        EC.presence_of_element_located((By.ID, "identifierId"))
    )
    email_input.send_keys("YOUR_EMAIL@gmail.com")

    # Click Next
    next_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//span[text()='Next']"))
    )
    next_btn.click()

    time.sleep(3)

    # Type password
    password_input = WebDriverWait(driver, 10).until(
        EC.presence_of_element_located((By.NAME, "Passwd"))
    )
    password_input.send_keys("YOUR_PASSWORD")

    # Click Next/Sign in
    signin_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//div[@class='VfPpkd-Jh9lGc']"))
    )
    signin_btn.click()

    print("Automation complete. Please complete 2FA verification manually.")
    print("The browser will remain open for manual interaction.")

    # Keep browser open for manual 2FA
    input("Press Enter after completing 2FA to continue...")

    # After 2FA, you can add additional automation here if needed
    print("Login process completed!")

except Exception as e:
    print(f"Error: {e}")

finally:
    # Comment out to keep browser open
    # driver.quit()
    pass