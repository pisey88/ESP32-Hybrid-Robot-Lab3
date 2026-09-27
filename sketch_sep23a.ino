#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include <ESP32Servo.h>

const char* BT_NAME = "Lab3Robot";

// ---------------- Motor pins ----------------
// Left-front:  IN 25/26, PWM ch4 -> pin 33
// Left-rear:   IN 27/32, PWM ch5 -> pin 14
// Right-front: IN 18/21, PWM ch6 -> pin 5
// Right-rear:  IN 22/23, PWM ch7 -> pin 19
// (Channels moved to 4-7 so the Servo library's auto-assigned
//  channel, usually 0-3, doesn't collide with the motors' timer.)

// ---------------- Servo ----------------
#define SERVO_PIN 17
Servo steeringServo;
int servoAngle = 85;              // start mid-range (30-140)
const int SERVO_MIN = 30;
const int SERVO_MAX = 140;
// ---------------- Ultrasonic (HC-SR04) ----------------
#define TRIG_PIN 13
#define ECHO_PIN 39
const float OBSTACLE_DISTANCE_CM = 30.0;

// ---------------- Modes ----------------
enum Mode { IDLE_MODE, MANUAL_MODE, AUTO_MODE };
Mode currentMode = IDLE_MODE;

enum AutoState { AUTO_FORWARD, AUTO_TURNING };
AutoState autoState = AUTO_FORWARD;
unsigned long turnStartTime = 0;
const unsigned long TURN_90_MS = 500;   // CALIBRATE on your robot
const int AUTO_FORWARD_SPEED = 25; // 0-100, slower forward speed
const int AUTO_TURN_SPEED    = 20; // 0-100, even slower turning speed


int speedPercent = 0;   // manual-mode joystick speed

// ---- edge-detect state (so a press only fires once, not every loop) ----
bool lastSquare = false, lastCircle = false, lastCross = false, lastTriangle = false;
bool lastSelect = false, lastStart = false;

void setup() {
  Serial.begin(115200);
  Dabble.begin(BT_NAME);

  pinMode(25, OUTPUT); pinMode(26, OUTPUT);
  ledcSetup(4, 20000, 8); ledcAttachPin(33, 4);

  pinMode(27, OUTPUT); pinMode(32, OUTPUT);
  ledcSetup(5, 20000, 8); ledcAttachPin(14, 5);

  pinMode(18, OUTPUT); pinMode(21, OUTPUT);
  ledcSetup(6, 20000, 8); ledcAttachPin(5, 6);

  pinMode(22, OUTPUT); pinMode(23, OUTPUT);
  ledcSetup(7, 20000, 8); ledcAttachPin(19, 7);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  steeringServo.attach(SERVO_PIN);
  steeringServo.write(servoAngle);

  stopMotors();
  Serial.println("Robot IDLE. Press SELECT for Manual, START for Automatic.");
}

void loop() {
  Dabble.processInput();

  // ---- Mode switching (edge-triggered so it fires once per press) ----
  bool selectBtn = GamePad.isSelectPressed();
  bool startBtn  = GamePad.isStartPressed();

  if (selectBtn && !lastSelect) {
    currentMode = MANUAL_MODE;
    stopMotors();
    Serial.println("Mode: MANUAL");
  }
  if (startBtn && !lastStart) {
    currentMode = AUTO_MODE;
    autoState = AUTO_FORWARD;
    stopMotors();
    Serial.println("Mode: AUTOMATIC");
  }
  lastSelect = selectBtn;
  lastStart  = startBtn;

  switch (currentMode) {
    case IDLE_MODE:   stopMotors();     break;
    case MANUAL_MODE: runManualMode();  break;
    case AUTO_MODE:   runAutoMode();    break;
  }
}

// ==================== MANUAL MODE ====================
void runManualMode() {
  int joyX = GamePad.getXaxisData();   // -7 to +7
  int joyY = GamePad.getYaxisData();
  const int DEADZONE = 1;

  // ---- Joystick drive ----
  if (abs(joyY) >= abs(joyX) && abs(joyY) > DEADZONE) {
    speedPercent = map(abs(joyY), 0, 7, 0, 100);
    uint8_t pwm = map(speedPercent, 0, 100, 0, 255);
    if (joyY > 0) moveForward(pwm); else moveBackward(pwm);
  }
  else if (abs(joyX) > DEADZONE) {
    speedPercent = map(abs(joyX), 0, 7, 0, 100);
    uint8_t pwm = map(speedPercent, 0, 100, 0, 255);
    if (joyX > 0) turnRight(pwm); else turnLeft(pwm);
  }
  else {
    stopMotors();
  }

  // ---- Servo control (edge-detected so one press = one step) ----
  bool sq = GamePad.isSquarePressed();
  bool ci = GamePad.isCirclePressed();
  bool cr = GamePad.isCrossPressed();
  bool tr = GamePad.isTrianglePressed();

  if (sq && !lastSquare) {
    servoAngle = constrain(servoAngle + 10, SERVO_MIN, SERVO_MAX);
    steeringServo.write(servoAngle);
    Serial.print("Servo -> "); Serial.println(servoAngle);
  }
  if (ci && !lastCircle) {
    servoAngle = constrain(servoAngle - 10, SERVO_MIN, SERVO_MAX);
    steeringServo.write(servoAngle);
    Serial.print("Servo -> "); Serial.println(servoAngle);
  }
  if (cr && !lastCross) {
    servoAngle = SERVO_MIN;
    steeringServo.write(servoAngle);
    Serial.println("Servo preset -> 30");
  }
  if (tr && !lastTriangle) {
    servoAngle = SERVO_MAX;
    steeringServo.write(servoAngle);
    Serial.println("Servo preset -> 140");
  }

  lastSquare = sq; lastCircle = ci; lastCross = cr; lastTriangle = tr;
}

// ==================== AUTOMATIC MODE ====================
void runAutoMode() {
  float distance = readDistanceCM();
  Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");

  uint8_t forwardPWM = map(AUTO_FORWARD_SPEED, 0, 100, 0, 255);
  uint8_t turnPWM     = map(AUTO_TURN_SPEED, 0, 100, 0, 255);

  switch (autoState) {
    case AUTO_FORWARD:
      if (distance < OBSTACLE_DISTANCE_CM) {
        Serial.println("Obstacle detected -> stopping, then rotating left ~90deg");
        stopMotors();
        delay(150);              // brief pause to kill forward momentum before turning
        autoState = AUTO_TURNING;
        turnStartTime = millis();
        turnLeft(turnPWM);
      } else {
        moveForward(forwardPWM);
      }
      break;

    case AUTO_TURNING:
      turnLeft(turnPWM);
      if (millis() - turnStartTime >= TURN_90_MS) {
        float d = readDistanceCM();
        if (d < OBSTACLE_DISTANCE_CM) {
          // still blocked -> keep rotating in another ~90deg increment
          turnStartTime = millis();
          Serial.println("Still blocked, continuing rotation");
        } else {
          autoState = AUTO_FORWARD;
          Serial.println("Path clear, resuming forward");
        }
      }
      break;
  }
}  

// ---- Ultrasonic distance in cm, 999 if no echo (treated as clear) ----
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // 30ms timeout, matches confirmed-working test
  if (duration == 0) return 999;
  return duration * 0.0343 / 2.0;
}

// ---- Motor helper functions ----
void stopMotors() {
  ledcWrite(4, 0); ledcWrite(5, 0); ledcWrite(6, 0); ledcWrite(7, 0);
}

void moveBackward(uint8_t speed) {
  digitalWrite(26, HIGH); digitalWrite(25, LOW);
  digitalWrite(32, HIGH); digitalWrite(27, LOW);
  digitalWrite(18, HIGH); digitalWrite(21, LOW);
  digitalWrite(22, HIGH); digitalWrite(23, LOW);
  ledcWrite(4, speed); ledcWrite(5, speed); ledcWrite(6, speed); ledcWrite(7, speed);
}

void moveForward(uint8_t speed) {
  digitalWrite(26, LOW); digitalWrite(25, HIGH);
  digitalWrite(32, LOW); digitalWrite(27, HIGH);
  digitalWrite(18, LOW); digitalWrite(21, HIGH);
  digitalWrite(22, LOW); digitalWrite(23, HIGH);
  ledcWrite(4, speed); ledcWrite(5, speed); ledcWrite(6, speed); ledcWrite(7, speed);
}

void turnLeft(uint8_t speed) {
  digitalWrite(26, HIGH); digitalWrite(25, LOW);
  digitalWrite(32, HIGH); digitalWrite(27, LOW);
  digitalWrite(18, LOW);  digitalWrite(21, HIGH);
  digitalWrite(22, LOW);  digitalWrite(23, HIGH);
  ledcWrite(4, speed); ledcWrite(5, speed); ledcWrite(6, speed); ledcWrite(7, speed);
}

void turnRight(uint8_t speed) {
  digitalWrite(26, LOW); digitalWrite(25, HIGH);
  digitalWrite(32, LOW); digitalWrite(27, HIGH);
  digitalWrite(18, HIGH); digitalWrite(21, LOW);
  digitalWrite(22, HIGH); digitalWrite(23, LOW);
  ledcWrite(4, speed); ledcWrite(5, speed); ledcWrite(6, speed); ledcWrite(7, speed);
}