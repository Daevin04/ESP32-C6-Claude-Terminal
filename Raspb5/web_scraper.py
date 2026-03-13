import time, serial
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

# === Serial setup ===
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)

# === Start Chromium ===
options = webdriver.ChromeOptions()
options.add_argument("--headless")   # use old headless mode for Pi compatibility
options.add_argument("--no-sandbox")
options.add_argument("--disable-dev-shm-usage")
options.add_argument("--disable-gpu")   # avoid GPU crashes on Pi
options.add_argument("--disable-software-rasterizer")
options.add_argument("--user-data-dir=/home/pi/selenium_profile")  # unique dir

# Use system ChromeDriver for ARM64
driver = webdriver.Chrome(service=Service("/usr/bin/chromedriver"), options=options)

try:
    print("Going to ChatGPT...")
    driver.get("https://chatgpt.com")
    time.sleep(3)

    # Check if already logged in
    try:
        ready_element = WebDriverWait(driver, 3).until(
            EC.presence_of_element_located((By.XPATH, "//div[contains(text(), 'Ready when you are.')]"))
        )
        print("Already logged in! Found 'Ready when you are.' message.")
        print("Login process completed!")
        time.sleep(3)
        driver.quit()
        exit()
    except:
        print("Not logged in yet, proceeding with login...")

    print("Looking for login button...")
    # Try multiple selectors for login button
    login_btn = None
    selectors = [
        "//button[@data-testid='login-button']",
        "//button[contains(text(), 'Log in')]",
        "//div[contains(text(), 'Log in')]",
        "//a[contains(text(), 'Log in')]"
    ]

    for selector in selectors:
        try:
            login_btn = WebDriverWait(driver, 5).until(
                EC.element_to_be_clickable((By.XPATH, selector))
            )
            print(f"Found login button with selector: {selector}")
            break
        except:
            continue

    if not login_btn:
        print("Login button not found. Page might already be logged in or structure changed.")
        print("Current page title:", driver.title)
        print("Current URL:", driver.current_url)
        driver.quit()
        exit()

    login_btn.click()
    print("Clicked login button")
    time.sleep(3)

    print("Looking for Google login...")
    google_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//button[contains(text(), 'Continue with Google')]"))
    )
    google_btn.click()
    print("Clicked Continue with Google")
    time.sleep(3)

    print("Entering email...")
    email_input = WebDriverWait(driver, 10).until(
        EC.presence_of_element_located((By.ID, "identifierId"))
    )
    email_input.send_keys("YOUR_EMAIL@gmail.com")
    print("Email entered")

    print("Clicking Next...")
    next_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//span[contains(text(), 'Next')]"))
    )
    next_btn.click()
    time.sleep(4)

    print("Entering password...")
    password_input = WebDriverWait(driver, 10).until(
        EC.presence_of_element_located((By.NAME, "Passwd"))
    )
    password_input.send_keys("YOUR_PASSWORD")
    print("Password entered")

    print("Clicking sign in...")
    signin_btn = WebDriverWait(driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//div[@class='VfPpkd-Jh9lGc']"))
    )
    signin_btn.click()
    print("Sign in clicked")

    print("Login steps completed. Checking for 2FA...")

    # Look for 2FA number and display it
    try:
        verification_number = WebDriverWait(driver, 15).until(
            EC.presence_of_element_located((By.XPATH, "//samp[contains(@class, 'Sevzkc')]"))
        )
        print(f"2FA VERIFICATION NUMBER: {verification_number.text}")
        print("Please verify this number on your device and complete 2FA.")
    except Exception as e:
        print("2FA number not found automatically. Please complete 2FA manually if needed.")
        print(f"2FA error: {e}")

    # Wait for manual 2FA completion
    input("Press Enter after completing 2FA to continue...")

    print("Login process completed!")
    time.sleep(3)

except Exception as e:
    print(f"Detailed error: {e}")
    print("Current page title:", driver.title)
    print("Current URL:", driver.current_url)

finally:
    driver.quit()