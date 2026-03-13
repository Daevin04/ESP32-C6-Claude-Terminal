#!/usr/bin/env python3
"""
Raspberry Pi 5 Claude Vision Analysis Server
Direct pipeline: Image → Claude Vision API → Q&A
Uses proven working Raspb5/test_chatgpt.py camera capture code
No OCR preprocessing - Claude handles image analysis directly
"""

from flask import Flask, request, jsonify
import os
import subprocess
import base64
import time
import requests

app = Flask(__name__)

# Claude API configuration
CLAUDE_API_KEY = os.environ.get("CLAUDE_API_KEY", "")
CLAUDE_API_URL = "https://api.anthropic.com/v1/messages"

def capture_image():
    """
    Capture image using proven working rpicam-still approach
    """
    print("📷 Capturing image...")

    image_path = "/tmp/claude_image.jpg"
    try:
        subprocess.run([
            "rpicam-still",
            "-o", image_path,
            "--width", "1920",
            "--height", "1080"
        ], check=True)
        print(f"✅ Image captured: {image_path}")
        return image_path
    except subprocess.CalledProcessError as e:
        raise Exception(f"❌ Failed to capture image: {e}")

def encode_image_to_base64(image_path):
    """
    Encode image to base64 for Claude API
    """
    with open(image_path, "rb") as image_file:
        image_data = image_file.read()
        base64_image = base64.b64encode(image_data).decode('utf-8')

    print(f"✅ Image encoded: {len(image_data)} bytes")
    return base64_image

def analyze_image_with_claude(base64_image):
    """
    Send image directly to Claude for analysis (no OCR preprocessing)
    """
    print("🤖 Analyzing image with Claude Vision...")

    headers = {
        "Content-Type": "application/json",
        "x-api-key": CLAUDE_API_KEY,
        "anthropic-version": "2023-06-01"
    }

    payload = {
        "model": "claude-3-5-sonnet-20241022",
        "max_tokens": 1000,
        "messages": [
            {
                "role": "user",
                "content": [
                    {
                        "type": "text",
                    "text": (
                        "Read the text in this image and answer any questions found. Respond ONLY with the direct answer content - no instructions, no explanations about how you're answering, no meta-commentary. If there's a question, give the pure factual answer. If there are multiple questions, answer each one directly. If there's no question, simply transcribe what the text says. If there is incomplete work, exercises, or fill-in-the-blank questions, complete the work and provide the missing answers. If there are any existing answers or work shown, check them for accuracy and provide corrections if they are wrong. Give complete, detailed answers but without any preamble or advice."
                        "\n\nCONTEXT: You are assisting with EGRE 365 Test 3 (VHDL). Base all reasoning on these notes:\n"
                        "[VHDL Delay Modeling]\n"
                        "- All digital signals are analog in reality.\n"
                        "- Rise/Fall time = 10%-90% (or 90%-10%).\n"
                        "- Propagation delay = input change -> output response.\n"
                        "- Inertial delay (default) filters short pulses.\n"
                        "- Transport delay passes all transitions.\n"
                        "- Reject clause ignores pulses shorter than its value.\n"
                        "[Structural VHDL]\n"
                        "- Describes how components connect.\n"
                        "- Elements: Declaration, Binding, Instantiation.\n"
                        "- Libraries: IEEE.STD_LOGIC_1164, work.<pkg>.all.\n"
                        "- Explicit method: Decl -> Bind -> Inst. Direct instantiation allowed.\n"
                        "[Generate]\n"
                        "- Repetitive structures via for-generate.\n"
                        "[Behavioral vs Structural]\n"
                        "- Behavioral: what it does. Structural: how it is wired. Top-level connects modules. Testbench drives/checks.\n"
                        "\nADDITIONAL CONTEXT: You are assisting with EGRE 347 Test 3 (C++ STL). Base all reasoning on these notes:\n"
                        "[C++ Containers]\n"
                        "- STL container types use class templates (generic types).\n"
                        "- Advantages: reusable, tested, efficient, type safe.\n"
                        "- Disadvantages: can be heavy vs bare arrays for tiny tasks.\n"
                        "[vector]\n"
                        "- Dynamic array, contiguous memory, O(1) index access.\n"
                        "- Methods: push_back, pop_back, size, clear, empty, at, front, back, insert, erase.\n"
                        "- Any type or class may be stored (must be copy/moveable as required).\n"
                        "- Inserts/removes in middle are O(n).\n"
                        "[iterators]\n"
                        "- begin, end, cbegin, cend; ++it forward; *it dereference.\n"
                        "- Range-for: for (auto& x : v) { ... }\n"
                        "[deque]\n"
                        "- Double-ended queue; fast push_front/push_back; not contiguous like vector.\n"
                        "[Algorithms]\n"
                        "- Non-modifying: find, count, all_of, any_of, none_of.\n"
                        "- Modifying: swap, replace, remove (use erase-remove idiom), transform.\n"
                        "- Sorting: sort, stable_sort, custom comparator (lambda).\n"
                        "- Min/Max: min, max, min_element, max_element.\n"
                        "\nTASK RULES:\n"
                        "1) Extract each question/problem from the image.\n"
                        "2) Provide complete final answers; finish any incomplete work.\n"
                        "3) If existing answers/work are present: if correct, label 'Correct.'; if wrong/incomplete, label 'Fix:' and provide the corrected result with a brief reason.\n"
                        "4) Show minimal but clear work (equations/short reasoning/code) sufficient to verify.\n"
                        "5) For math, use LaTeX wrapped in $$...$$. For code, use fenced blocks (```vhdl``` or ```cpp```).\n"
                        "6) If VHDL delays appear, explicitly name inertial/transport/reject and the effect on short pulses. If STL questions appear, name the specific container/algorithm and show correct usage with iterators.\n"
                        "7) Keep output self-contained, strictly exam-focused, and with no preamble or meta commentary.\n"
                        "\nOUTPUT FORMAT (strict):\n"
                        "Q1: <extracted question>\n"
                        "A1: <final answer>\n"
                        "Work:\n"
                        "<steps/equations/code>\n\n"
                        "Q2: <extracted question>\n"
                        "A2: <final answer>\n"
                        "Work:\n"
                        "<steps/equations/code>\n"
                    )


                    },
                    {
                        "type": "image",
                        "source": {
                            "type": "base64",
                            "media_type": "image/jpeg",
                            "data": base64_image
                        }
                    }
                ]
            }
        ]
    }

    try:
        response = requests.post(CLAUDE_API_URL, headers=headers, json=payload, timeout=30)

        if response.status_code == 200:
            result = response.json()
            analysis = result['content'][0]['text']
            print(f"✅ Claude analysis received: {len(analysis)} characters")
            return analysis
        else:
            print(f"❌ Claude API error: {response.status_code}")
            print(f"❌ Response: {response.text}")
            return f"Error: Claude API returned status {response.status_code}"

    except Exception as e:
        print(f"❌ Claude analysis error: {e}")
        return f"Error analyzing image: {str(e)}"

@app.route('/analyze_photo', methods=['POST'])
def analyze_photo():
    """
    Main endpoint: Image → Claude Vision pipeline (no OCR)
    """
    try:
        print(f"\n📱 Claude Vision request from ESP32-C6: {request.remote_addr}")

        # Stage 1: Capture image
        image_path = capture_image()

        # Stage 2: Encode image for Claude
        base64_image = encode_image_to_base64(image_path)

        # Stage 3: Analyze with Claude Vision
        analysis = analyze_image_with_claude(base64_image)

        # Return result
        result = {
            "success": True,
            "analysis": analysis
        }

        print("✅ Claude Vision pipeline complete - sending to ESP32-C6\n")
        return jsonify(result)

    except Exception as e:
        print(f"❌ Error in Claude Vision pipeline: {e}")
        return jsonify({
            "success": False,
            "error": f"Pipeline failed: {str(e)}"
        }), 500

@app.route('/status', methods=['GET'])
def status():
    """Health check endpoint"""
    # Test camera availability
    try:
        subprocess.run(["rpicam-still", "--version"],
                      capture_output=True, check=True, timeout=5)
        camera_status = "ready"
    except Exception:
        camera_status = "not available"

    # Test Claude API availability
    try:
        headers = {
            "Content-Type": "application/json",
            "x-api-key": CLAUDE_API_KEY,
            "anthropic-version": "2023-06-01"
        }
        test_payload = {
            "model": "claude-3-5-sonnet-20241022",
            "max_tokens": 10,
            "messages": [{"role": "user", "content": "Test"}]
        }
        test_response = requests.post(CLAUDE_API_URL, headers=headers, json=test_payload, timeout=10)
        claude_status = "ready" if test_response.status_code == 200 else "error"
    except Exception:
        claude_status = "not available"

    return jsonify({
        "status": "running",
        "camera": camera_status,
        "claude_api": claude_status,
        "model": "Arducam IMX708",
        "server": "Pi 5 Claude Vision Analysis Server",
        "pipeline": "Image → Claude Vision API → Q&A"
    })

@app.route('/', methods=['GET'])
def home():
    """Simple home page"""
    return """
    <h1>🍓 Pi 5 Claude Vision Analysis Server</h1>
    <p><strong>Pipeline:</strong> Image → Claude Vision API → Q&A</p>
    <p><strong>Camera:</strong> Arducam IMX708 Wide</p>
    <p><strong>AI:</strong> Claude 3.5 Sonnet (vision capable)</p>
    <p><strong>OCR:</strong> Disabled - Claude handles text recognition directly</p>

    <h2>API Endpoints:</h2>
    <ul>
        <li><strong>POST /analyze_photo</strong> - Capture and analyze with Claude</li>
        <li><strong>GET /status</strong> - Server, camera, and Claude API status</li>
    </ul>

    <h2>Direct Vision Processing:</h2>
    <ol>
        <li><strong>Stage 1:</strong> Camera captures high-resolution image</li>
        <li><strong>Stage 2:</strong> Image sent directly to Claude Vision API</li>
        <li><strong>Stage 3:</strong> Claude extracts text and provides Q&A responses</li>
    </ol>

    <h2>Benefits of Claude Vision:</h2>
    <ul>
        <li><strong>Better accuracy:</strong> Advanced vision model vs OCR preprocessing</li>
        <li><strong>Simpler pipeline:</strong> No intermediate OCR step required</li>
        <li><strong>Better understanding:</strong> Context-aware text analysis</li>
        <li><strong>Faster processing:</strong> Single API call instead of OCR + LLM</li>
    </ul>

    <h2>Hardware Setup:</h2>
    <ul>
        <li>ESP32-C6 → Serial/HTTP requests to Pi</li>
        <li>Raspberry Pi 5 → Camera + Claude Vision processing</li>
        <li>Shared network (USB serial or local Wi-Fi)</li>
    </ul>
    """

if __name__ == '__main__':
    print("🍓 Pi 5 Claude Vision Analysis Server")
    print("======================================")
    print(f"🤖 Claude API: {'✅ Configured' if CLAUDE_API_KEY else '❌ Missing'}")

    # Test camera
    try:
        subprocess.run(["rpicam-still", "--list-cameras"],
                      capture_output=True, text=True, timeout=10)
        print("📷 Camera: ✅ rpicam-still available")
    except Exception as e:
        print(f"📷 Camera: ❌ Not available ({e})")

    # Test Claude API
    try:
        headers = {
            "Content-Type": "application/json",
            "x-api-key": CLAUDE_API_KEY,
            "anthropic-version": "2023-06-01"
        }
        test_payload = {
            "model": "claude-3-5-sonnet-20241022",
            "max_tokens": 10,
            "messages": [{"role": "user", "content": "Test"}]
        }
        test_response = requests.post(CLAUDE_API_URL, headers=headers, json=test_payload, timeout=10)
        if test_response.status_code == 200:
            print("🤖 Claude API: ✅ Available")
        else:
            print(f"🤖 Claude API: ⚠️ Error {test_response.status_code}")
    except Exception as e:
        print(f"🤖 Claude API: ❌ Not available ({e})")

    print(f"🌐 Server: Starting on 0.0.0.0:5000")
    print(f"📱 Ready for ESP32-C6 requests")
    print("🔄 Pipeline: Image → Claude Vision → Q&A")
    print("⚠️  OCR disabled - using direct vision analysis")
    print("======================================\n")

    # Start Flask server
    app.run(host='0.0.0.0', port=5000, debug=True)
