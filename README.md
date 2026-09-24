ESP32 Hybrid Robot Control Using Dabble and Ultrasonic Sensor — Lab 3

Course: ICT 361 Submission Type: Group
Group Members

LY SOKPISEY
LINH KIMSOURMANA


Objective

This project implements a hybrid robot control system on an ESP32 development board. The robot can operate in two distinct modes: Manual Mode (remote control via the Dabble smartphone app over Bluetooth) and Automatic Mode (autonomous navigation using an ultrasonic sensor for obstacle avoidance). The robot remains idle at boot and requires explicit user input via the Dabble app to activate a mode. In Manual Mode, joystick commands control 4-wheel movement, while controller buttons adjust a front-mounted servo motor within a strict $30^\circ$ to $140^\circ$ range. In Automatic Mode, the robot navigates autonomously, automatically detecting obstacles within 20 cm and turning left in $90^\circ$ increments until a clear path is found.


Hardware Setup

Input / Output | GPIO / Module | Function
--- | --- | ---
Dabble App (Bluetooth) | ESP32 Bluetooth Module | Mode selection, movement joystick, servo button inputs
Ultrasonic Sensor TRIG | GPIO 5 | Output trigger pin for distance measurement
Ultrasonic Sensor ECHO | GPIO 18 | Input echo pin for distance measurement
Servo Motor | GPIO 13 | Front-mounted position control signal

Motor Driver Output | GPIO (Dir A / Dir B) | Motor
--- | --- | ---
Left Motors (Channel A) | GPIO 16 / GPIO 17 | Motor 1 & Motor 2
Right Motors (Channel B) | GPIO 18 / GPIO 19 | Motor 3 & Motor 4


Control Logic Summary

1. Startup Safety & Mode Selection: The robot initializes in an IDLE state with all motors stopped. Pressing the SELECT button on the Dabble GamePad activates Manual Mode, while pressing the START button activates Automatic Mode.
2. Manual Directional Control: In Manual Mode, joystick directional inputs trigger 4-wheel motion (Up: Forward, Down: Backward, Left: Rotate Left, Right: Rotate Right). Releasing the controls stops all motors.
3. Servo Limit Control: Servo angle is adjusted incrementally or via presets (+10° via Square, -10° via Circle, 30° via Cross, 140° via Triangle). All angles are strictly clamped between $30^\circ$ and $140^\circ$.
4. Automatic Obstacle Avoidance: In Automatic Mode, joystick commands are ignored. An ultrasonic sensor periodically checks for obstacles. If an object is detected within 20 cm, the robot stops, rotates left by approximately $90^\circ$, and continues checking iteratively until a clear path is found before proceeding forward.


Flowchart



Demo Video
[![Watch Demonstration Video](https://img.youtube.com/vi/YOUR_VIDEO_ID_HERE/0.jpg)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID_HERE)


Explanation

1. Purpose of Hybrid System Architecture and State Machine

Real-world mobile robotics rarely relies on purely manual or purely autonomous control; instead, modern systems integrate both so humans can remotely intervene when needed while allowing autonomous algorithms to take over routine navigation tasks. In this project, state machine logic (`IDLE`, `MANUAL`, `AUTOMATIC`) decouples control modes cleanly. Operating inside a single continuous loop, the ESP32 handles inputs without long blocking delays, ensuring that pressing the SELECT or START button immediately switches the operating mode without needing a system reset or causing unpredictable motor behavior. The initial `IDLE` state serves as a safety feature to ensure the robot never moves immediately upon power-up until explicit user command is confirmed.

2. Servo Angle Constraining and Incremental Control

Servo motors attached to mechanical linkages or steering mechanisms must be protected from over-rotation, which can cause physical binding, gear stripping, or high electrical current draw. The control software enforces strict soft limits using `constrain(angle, 30, 140)`, guaranteeing that no matter how many times incremental step buttons (Square for $+10^\circ$, Circle for $-10^\circ$) or preset positioning buttons (Cross for $30^\circ$, Triangle for $140^\circ$) are pressed, the target position never violates the $30^\circ$ to $140^\circ$ structural boundaries. Incorporating small non-blocking debounce delays prevents single button taps from rapidly cascading into multiple unwanted step increments.

3. Obstacle Avoidance Logic and Non-Blocking Timing

Autonomous navigation relies on a continuous distance-sensing loop using the front-mounted ultrasonic sensor. Using non-blocking `millis()` timing checks rather than continuous heavy delay cycles allows the sensor to measure clearance without freezing the controller's main processing loop. When an obstacle drops below the 20 cm safety threshold, the state logic halts forward driving immediately to prevent collision, executes a timed differential turn left of approximately $90^\circ$, and pauses briefly for distance stabilization. If the new path remains blocked, the robot executes another $90^\circ$ left increment, repeating this process iteratively until a clear heading is determined.
