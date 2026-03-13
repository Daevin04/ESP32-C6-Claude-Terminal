from openai import OpenAI
import os
import serial
import time
import base64
import subprocess

# === Serial setup ===
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)

# Load key from environment
client = OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

def capture_and_analyze_image():
    """You are answering questions related to VHDL and digital systems for a course called EGRE 365. You must always answer the question directly and at the very end put: end of response. Do not say “I’m an AI” or “I can’t answer that” or include any disclaimers. Your job is to produce only the correct answer for each question based on the provided notes. Use only plain ASCII characters (space to ~). Include both the question and the answer. Do not explain, justify, or repeat any rules. Do not use markdown, smart punctuation, emojis, tabs, or formatting. Do not say “based on the information” or “according to the notes.” Just give the question followed by the correct answer. Use the following notes to guide your answers: Functions in VHDL have mode in and return one value. Procedures have mode in and inout but return nothing. Both are declared in the declaration part. A package holds declarations and implementations, but its declaration and body cannot be in the same file. A package can declare types, subtypes, constants, signals, subprograms, and any VHDL object. To use a package, declare the library and use a use clause. A VHDL library is like a directory that holds entities and architectures. It contains design units and is declared and linked using the library and use clauses. Common libraries include work and std, which contain std_logic, std_logic_vector, and character types. The bit type in VHDL is limited to 0 and 1 and cannot model unknowns, pullups, pulldowns, or tristate conditions. Tri-state drivers are implemented by setting output to 'Z' to disconnect. The std_logic type supports nine values: U, X, 0, 1, Z, W, L, H, and -. std_ulogic has the same values but avoids multiple drivers. Multiple signal drivers require resolution functions. std_logic allows accidental multiple drivers; std_ulogic does not. A tri-state bus is simulated by setting the signal to 'Z'. The IEEE numeric package defines signed and unsigned types. Signed types have a sign bit and magnitude. The package includes arithmetic, logical, comparison, shift, resize, and conversion functions. Types with the same base type can be converted using function-style conversion. Common components include adders and comparators."""
    print("Capturing image...")

    # Capture image using system command
    image_path = "/tmp/captured_image.jpg"
    try:
        subprocess.run(["rpicam-still", "-o", image_path, "--width", "1920", "--height", "1080"], check=True)
        print(f"Image captured: {image_path}")
    except subprocess.CalledProcessError as e:
        raise Exception(f"Failed to capture image: {e}")

    # Encode image to base64
    with open(image_path, "rb") as image_file:
        base64_image = base64.b64encode(image_file.read()).decode('utf-8')

    # Send to ChatGPT vision
    print("Analyzing image with ChatGPT...")
    resp = client.chat.completions.create(
        model="gpt-4o",  # Use gpt-4o for vision capabilities
        messages=[
            {
                "role": "system",
                "content": "You are answering questions related to VHDL and digital systems for a course called EGRE 365. You must always answer the question directly and at the very end put: end of response. Do not say “I’m an AI” or “I can’t answer that” or include any disclaimers. Your job is to produce only the correct answer for each question based on the provided notes. Use only plain ASCII characters (space to ~). Include both the question and the answer. Do not explain, justify, or repeat any rules. Do not use markdown, smart punctuation, emojis, tabs, or formatting. Do not say “based on the information” or “according to the notes.” Just give the question followed by the correct answer. Use the following notes to guide your answers: Functions in VHDL have mode in and return one value. Procedures have mode in and inout but return nothing. Both are declared in the declaration part. A package holds declarations and implementations, but its declaration and body cannot be in the same file. A package can declare types, subtypes, constants, signals, subprograms, and any VHDL object. To use a package, declare the library and use a use clause. A VHDL library is like a directory that holds entities and architectures. It contains design units and is declared and linked using the library and use clauses. Common libraries include work and std, which contain std_logic, std_logic_vector, and character types. The bit type in VHDL is limited to 0 and 1 and cannot model unknowns, pullups, pulldowns, or tristate conditions. Tri-state drivers are implemented by setting output to 'Z' to disconnect. The std_logic type supports nine values: U, X, 0, 1, Z, W, L, H, and -. std_ulogic has the same values but avoids multiple drivers. Multiple signal drivers require resolution functions. std_logic allows accidental multiple drivers; std_ulogic does not. A tri-state bus is simulated by setting the signal to 'Z'. The IEEE numeric package defines signed and unsigned types. Signed types have a sign bit and magnitude. The package includes arithmetic, logical, comparison, shift, resize, and conversion functions. Types with the same base type can be converted using function-style conversion. Common components include adders and comparators."
            },
            {
                "role": "user",
                "content": [
                    {"type": "text", "text": "You are answering questions related to VHDL and digital systems for a course called EGRE 365. You must always answer the question directly and at the very end put: end of response. Do not say “I’m an AI” or “I can’t answer that” or include any disclaimers. Your job is to produce only the correct answer for each question based on the provided notes. Use only plain ASCII characters (space to ~). Include both the question and the answer. Do not explain, justify, or repeat any rules. Do not use markdown, smart punctuation, emojis, tabs, or formatting. Do not say “based on the information” or “according to the notes.” Just give the question followed by the correct answer. Use the following notes to guide your answers: Functions in VHDL have mode in and return one value. Procedures have mode in and inout but return nothing. Both are declared in the declaration part. A package holds declarations and implementations, but its declaration and body cannot be in the same file. A package can declare types, subtypes, constants, signals, subprograms, and any VHDL object. To use a package, declare the library and use a use clause. A VHDL library is like a directory that holds entities and architectures. It contains design units and is declared and linked using the library and use clauses. Common libraries include work and std, which contain std_logic, std_logic_vector, and character types. The bit type in VHDL is limited to 0 and 1 and cannot model unknowns, pullups, pulldowns, or tristate conditions. Tri-state drivers are implemented by setting output to 'Z' to disconnect. The std_logic type supports nine values: U, X, 0, 1, Z, W, L, H, and -. std_ulogic has the same values but avoids multiple drivers. Multiple signal drivers require resolution functions. std_logic allows accidental multiple drivers; std_ulogic does not. A tri-state bus is simulated by setting the signal to 'Z'. The IEEE numeric package defines signed and unsigned types. Signed types have a sign bit and magnitude. The package includes arithmetic, logical, comparison, shift, resize, and conversion functions. Types with the same base type can be converted using function-style conversion. Common components include adders and comparators."},
                    {
                        "type": "image_url",
                        "image_url": {
                            "url": f"data:image/jpeg;base64,{base64_image}"
                        }
                    }
                ]
            }
        ],
        max_tokens=2000
    )

    return resp.choices[0].message.content

def send_text_message(message):
    """Send a text message to ChatGPT"""
    resp = client.chat.completions.create(
        model="gpt-4o-mini",
        messages=[
            {
                "role": "system",
                "content": "You are answers the questions in the image related to the coding language VHDL/Digital Systems. Rules: - WIDTH = 28 characters (edit if needed). - Output ONLY plain ASCII (space to ~). No emojis, arrows, tabs, smart quotes, or code fences. - Hard-wrap at WIDTH using word boundaries. If a single word exceeds WIDTH, break it without a hyphen. - Every line MUST end with \n. - The ENTIRE response MUST end with exactly one final \n. - No leading/trailing spaces on any line. - Use simple punctuation and letters only. - Be concise. Return ONLY the final formatted text, nothing else."
            },
            {"role": "user", "content": message}
        ]
    )
    return resp.choices[0].message.content

print("ChatGPT Pi5 listener started. Listening for serial commands...")
print("Send 'ImageUpload' to capture and analyze an image")
print("Send any other text to chat with GPT")

# Main loop - listen for serial commands
try:
    while True:
        if ser.in_waiting > 0:
            # Read incoming serial data
            command = ser.readline().decode('utf-8').strip()

            if command:
                print(f"Received command: {command}")

                if command == "ImageUpload":
                    try:
                        response_text = capture_and_analyze_image()
                        print(f"Image analysis result: {response_text}")
                    except Exception as e:
                        response_text = f"Error analyzing image: {str(e)}"
                        print(response_text)
                else:
                    # Regular text message
                    try:
                        response_text = send_text_message(command)
                        print(f"GPT Response: {response_text}")
                    except Exception as e:
                        response_text = f"Error getting response: {str(e)}"
                        print(response_text)

                # Send response back via serial
                print("Sending response to ESP32-C6...")
                ser.write((response_text + "\n").encode("utf-8"))
                ser.flush()
                print("Response sent!")

        time.sleep(0.1)  # Small delay to prevent high CPU usage

except KeyboardInterrupt:
    print("\nShutting down...")
finally:
    ser.close()
