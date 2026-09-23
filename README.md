# ESP32 IoT Robotic Arm & Tendon-Driven Soft Gripper

An IoT-controlled robotic arm featuring a continuous-rotation pan/tilt base and a custom three-finger, tendon-driven mechanical soft gripper. The system is driven by an ESP32 and controlled remotely via the Blynk IoT platform.

## ⚙️ Hardware Architecture
* **Microcontroller:** ESP32
* **Actuation (Pan/Tilt):** Continuous Rotation Servos
* **Actuation (Gripper):** N20 Micro Gearmotor with Magnetic Encoder
* **Motor Driver:** TB6612FNG (For N20 bidirectional control)
* **Gripper Mechanism:** 3D-printed TPU mechanically-driven soft fingers, actuated by the N20 motor rotating a worm gear and follower.

## 💻 Software & Control Logic
The firmware is written in C++ and leverages `BlynkSimpleEsp32` for remote telemetry and control.

* **Asymmetric Joystick Mapping (Tilt Arm):** Continuous servos fighting gravity require highly specific PWM signals. The tilt arm uses an asymmetric mapping curve to distinguish between lifting against gravity, active braking, and powered descent.
* **Closed-Loop Gripper Control:** The N20 motor utilizes a custom PID-style interrupt loop reading the quadrature encoder. This allows the system to target precise rotational clicks (e.g., exactly 3 turns to fully close the tendons) and actively lock the brakes when target is reached.

## 📂 Repository Contents
* `/CAD` - Contains the SolidWorks part and assembly files (`.SLDPRT`, `.SLDASM`) for the arm and the TPU soft gripper, alongside `.STL` exports for immediate 3D printing.
* `/Firmware` - The Arduino IDE / PlatformIO compatible C++ code for the ESP32.

## 🚀 Setup & Installation
1. Flash the firmware to the ESP32.
2. Update the `ssid`, `pass`, and `BLYNK_AUTH_TOKEN` variables with your local credentials.
3. In the Blynk App, configure the datastreams:
   * **V0 (Joystick X):** Map to `-100 to 100` (Pan Base)
   * **V1 (Joystick Y):** Map to `-100 to 100` (Tilt Arm)
   * **V2 (Switch):** Map to `0 and 100` (Gripper Open/Close)

---
**Author:* * Aadi Shah | B.Tech Mechanical Engineering, IIT Ropar
