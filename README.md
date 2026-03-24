Gesture-Controlled Robot

A real-time, vision-based robot control system that uses *hand gesture recognition* to wirelessly drive a differential-drive robot. Finger count detected via a webcam is transmitted over *UDP/Wi-Fi* to an *ESP32* microcontroller, which maps each gesture to a motor command.

Table of Contents

- [Overview]
- [System Architecture]
- [Features]
- [Hardware Requirements]
- [Software Requirements]
- [Project Structure]
- [Setup & Usage]
- [Gesture-to-Command Mapping]
- [Technical Details]
- [Known Limitations]
- [Future Improvements]

Overview

This project implements a *contactless robot control interface* using computer vision. A Python script running on a host PC captures live webcam feed, detects hand landmarks using *MediaPipe*, counts the number of extended fingers, and transmits this value as a UDP packet to an ESP32 operating as a *Wi-Fi Access Point*. The ESP32 decodes the received integer and drives two DC motors via a *TB6612FNG motor driver* accordingly.

System Architecture

┌─────────────────────────────────────┐        Wi-Fi (UDP)         ┌──────────────────────────────┐
│           HOST PC (Python)          │ ─────────────────────────▶│        ESP32 (Arduino)        │
│                                     │    IP: 192.168.4.1         │                              │
│  Webcam → MediaPipe → Finger Count  │    Port: 4210              │  UDP Listener → Motor Driver │
│         (GetData.py)                │                            │       (main.cpp)             │
└─────────────────────────────────────┘                            └──────────────────────────────┘
                                                                            │
                                                                   ┌────────┴────────┐
                                                                   │  Motor A  Motor B│
                                                                   │  (Left)  (Right) │
                                                                   └─────────────────┘

The ESP32 creates its own *SoftAP (Access Point)* network. The host PC connects to this network, and all communication happens over a local UDP socket — no internet connection or router required.

Features

- *Real-time hand tracking* using MediaPipe Hands at 80% detection and tracking confidence
- *Hand chirality awareness* — thumb detection logic correctly handles both left and right hands
- *Wireless control* over ESP32-hosted Wi-Fi (no external router needed)
- *UDP communication* for low-latency, connectionless data transfer
- *Dual-channel PWM motor control* using the ESP32's `ledcWrite` API (1 kHz, 8-bit resolution)
- *Fail-safe behaviour* — robot automatically stops if no UDP packet is received within a loop cycle
- Live *OpenCV overlay* showing detected finger count on the webcam feed

Hardware Requirements

Component - Details 

Microcontroller - ESP32 (any variant with Wi-Fi) 
Motor Driver - TB6612FNG (or compatible dual H-bridge)
DC Motors - 2× (differential drive configuration)
Power Supply - Suitable battery pack for motors + ESP32
Host PC - Any machine with a webcam and Python support 

Pin Mapping (ESP32 → TB6612FNG)

Signal - ESP32 GPIO

AIN1 - 26 
AIN2 - 25 
PWMA - 33 
BIN1 - 14 
BIN2 - 12 
PWMB - 13 
STBY - 27 


Software Requirements

# Host PC (Python)

- Python 3.7+
- OpenCV (`cv2`)
- MediaPipe

Install dependencies:
bash -
pip install opencv-python mediapipe


# ESP32 (PlatformIO / Arduino IDE)

- Arduino framework for ESP32
- Built-in libraries: `WiFi.h`, `WiFiUdp.h`

> Tested with PlatformIO. If using Arduino IDE, ensure the ESP32 board package is installed!

Project Structure

gesture-controlled-robot/
│
├── GetData.py        # Host-side: webcam capture, hand detection, UDP transmitter
└── main.cpp          # ESP32-side: Wi-Fi AP setup, UDP receiver, motor control

Setup & Usage

# Step 1 — Flash the ESP32

1. Open `main.cpp` in PlatformIO or Arduino IDE.
2. Set your desired AP credentials:
   cpp -
   const char* ssid = "NAME_OF_AP";
   const char* password = "PASSWORD_AP";
3. Flash the firmware to the ESP32 and open the Serial Monitor (9600 baud) to confirm the AP IP address (`192.168.4.1`).

# Step 2 — Connect the Host PC to the ESP32's Wi-Fi

Connect your PC to the Wi-Fi network created by the ESP32 using the credentials set above.

# Step 3 — Run the Python Script

bash -
python GetData.py

The webcam feed will open with an overlay showing the detected finger count. Show your hand to the camera and use gestures to control the robot.

Press `Q` to quit.

# Gesture-to-Command Mapping

Fingers Extended - Command

0 - *Stop* 
1 - *Forward*
2 - *Reverse*
3 - *Turn Left*
4 - *Turn Right*
5 - *Stop* (fallback)

> All movement commands run motors at full speed (PWM duty cycle = 255/255).

---

Technical Details

# Hand Landmark Detection (`GetData.py`)

MediaPipe Hands returns 21 landmarks per detected hand. Finger extension is evaluated as follows:

- *Index, Middle, Ring, Pinky*: A finger is considered extended if its *tip landmark (y)* is above its *PIP joint landmark (y)* in image coordinates (i.e., `tip.y < pip.y`).
- *Thumb*: Extension is determined by the *x-axis* relationship between the tip (landmark 4) and IP joint (landmark 2), with logic inverted for left vs. right hand to account for mirroring.

The webcam feed is horizontally flipped (`cv2.flip(frame, 1)`) to provide a mirror-view interface.

# UDP Communication

- *Protocol*: UDP (connectionless, low-latency)
- *Payload*: A single ASCII-encoded integer string (`"0"` through `"5"`)
- *Target*: `192.168.4.1:4210` (ESP32 SoftAP address)

# Motor Control (`main.cpp`)

- PWM is configured using ESP32's `ledcSetup` / `ledcAttachPin` API on channels 0 and 1 at *1 kHz, 8-bit resolution*.
- The TB6612FNG `STBY` pin is driven `HIGH` during motion and `LOW` on stop.
- Motor direction is determined by the `AIN1/AIN2` and `BIN1/BIN2` logic-level combinations.
- A *100 ms delay* is applied after each command execution to debounce rapid packet bursts.

---

Known Limitations

- *Fixed PWM speed* — all motion commands use maximum speed (255); no variable speed control based on gesture confidence or position.
- *Single-hand detection only* — `max_num_hands=1` is enforced in MediaPipe config.
- *No packet acknowledgement* — UDP is fire-and-forget; dropped packets cause the robot to stop (which acts as a passive fail-safe).
- *Lighting sensitivity* — MediaPipe hand detection performance degrades under poor or inconsistent lighting.
- *No obstacle avoidance* — the robot has no sensors; collision prevention is entirely the operator's responsibility.

---

Future Improvements

- [ ] Variable speed control using hand distance from camera (depth estimation)
- [ ] Replace finger-count mapping with custom gesture classifier (e.g., fist = stop, open palm = forward)
- [ ] Add WebSocket or TCP fallback for environments with high packet loss
- [ ] Integrate ultrasonic/IR sensors on the ESP32 for basic obstacle avoidance
- [ ] Add an OLED display on the robot to echo received commands locally
- [ ] OTA (Over-the-Air) firmware update support via ESP32's built-in OTA library
