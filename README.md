# Lab 3: Hybrid Robot Control Using Dabble and Ultrasonic Sensor

Course: ICT 361 Submission Type: Group
Group Members

LY SOKPISEY
LINH KIMSOURMANA

## Overview
This project implements a dual-mode hybrid control system for a 4-motor robotic vehicle using an **ESP32**, **Dabble Bluetooth App**, **Servo Motor**, and an **Ultrasonic Sensor**. The robot operates in two primary execution modes—**Manual Mode** and **Automatic Mode**—and remains safely in an **Idle State** upon initial system boot.

---

## System Architecture & Hardware Components
* **Controller:** ESP32 Development Board
* **Actuators:** Motor Driver (4 DC Motors), 1 Servo Motor
* **Sensors:** Ultrasonic Sensor (HC-SR04, front-facing)
* **Connectivity:** Bluetooth via Dabble Smartphone App
* **Power:** External Battery Pack

---

## Operational Logic & Software Design

### 1. System Initialization & Idle State
Upon boot-up, the system initializes all hardware peripherals:
* Digital I/O pins for the motor driver and ultrasonic sensor.
* Servo attachment and Dabble Bluetooth protocol initialization via `Dabble.begin()`.
* **Idle Safety Protocol:** The system defaults to `IDLE` mode. All motor outputs are set to LOW (stopped), and joystick inputs are ignored until a mode selection key (`SELECT` or `START`) is registered.

### 2. Mode Switching Logic
The main execution loop continuously monitors incoming Dabble Bluetooth packets using `Dabble.processInput()`:
* **`SELECT` Button:** Switches the robot into **Manual Mode**.
* **`START` Button:** Switches the robot into **Automatic Mode**.

---

## Detailed Control Modes

### Manual Control Mode
In Manual Mode, the robot provides real-time control over motor propulsion and servo positioning:

* **Movement Control (Joystick):**
  * **Up:** Drive Forward
  * **Down:** Drive Backward
  * **Left:** Rotate Left (In-place turn)
  * **Right:** Rotate Right (In-place turn)

* **Servo Angle Control:**
  * **Square Button:** Increments servo angle by **+10°**.
  * **Circle Button:** Decrements servo angle by **-10°**.
  * **Cross Button:** Moves servo to fixed preset angle of **30°**.
  * **Triangle Button:** Moves servo to fixed preset angle of **140°**.

* **Angle Clamping & Safety Protection:**
  To protect hardware from structural strain, all requested angles pass through a soft limit function:
  $$\text{Angle} = \text{constrain}(\text{Angle}, 30, 140)$$
  This guarantees the physical angle stays strictly within the **30° to 140°** boundary regardless of rapid or repeated button presses.

---

### Automatic Obstacle Avoidance Mode
In Automatic Mode, joystick controls are bypassed, and the robot relies autonomously on real-time ultrasonic sensor feedback:

1. **Distance Measurement:** The ultrasonic sensor emits acoustic pulses to calculate distance to target objects ahead in centimeters.
2. **Threshold Verification:**
   * **Path Clear ($\ge 20\text{ cm}$):** Robot continuously drives straight forward.
   * **Obstacle Detected ($< 20\text{ cm}$):** The robot executes an avoidance maneuver:
     1. Halts forward motion.
     2. Rotates left by approximately **90°**.
     3. Re-evaluates distance on the next loop cycle.
3. **Continuous Re-evaluation Loop:** If an obstacle is still detected within $20\text{ cm}$ after turning, the robot turns left again in $\sim90^\circ$ increments until a clear path is registered, at which point forward movement resumes.

---

## Performance & Optimization Notes
* **Non-Blocking Logic:** The program architecture minimizes long `delay()` statements within the main loop to ensure smooth Bluetooth polling and immediate reaction to sudden obstacles.
* **State Isolation:** Mutual exclusion between states prevents conflicting motor commands during mode switches.


Flowchart

<img width="815" height="1071" alt="ESP32-Hybrid-Robot-Lab3" src="https://github.com/user-attachments/assets/40eaeab7-0181-48bd-baed-6a604d985a78" />



Demo Video
[![Watch Demonstration Video](https://[img.youtube.com/vi/YOUR_VIDEO_ID_HERE/0.jpg](https://drive.google.com/file/d/1XPZKZyRJcN9jO48Y0igzZ3v8CPvSvFVp/view?usp=sharing))]([https://www.youtube.com/watch?v=YOUR_VIDEO_ID_HERE](https://drive.google.com/file/d/1XPZKZyRJcN9jO48Y0igzZ3v8CPvSvFVp/view?usp=sharing))
