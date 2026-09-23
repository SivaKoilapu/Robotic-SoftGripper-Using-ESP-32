#define BLYNK_TEMPLATE_ID "TMPL3nZPT9_pz"
#define BLYNK_TEMPLATE_NAME "Gripper Model with 360 Arm"
#define BLYNK_AUTH_TOKEN "63S8JMw-MCjBy4H8H5HToyI-dCxdp5cI"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

char ssid[] = "O";
char pass[] = "password";

// --- SERVO PINS & OBJECTS ---
Servo servo1; // V0 Control (Pan Base)
Servo servo2; // V1 Control (Tilt Arm)
const int servo1Pin = 13;
const int servo2Pin = 25;

// If a servo "creeps" when the joystick is at 0
int neutralPoint = 95; 

// --- N20 MOTOR PINS (TB6612FNG) ---
const int AIN1 = 26;
const int AIN2 = 27;
const int PWMA = 14;

// --- ENCODER PINS ---
const int ENCODER_C1 = 32;
const int ENCODER_C2 = 33;

// --- CALIBRATED GRIPPER MATH ---
volatile long encoderCount = 0; // Live position
long targetPosition = 0; // Desired position

// Your exact hardware numbers!
float n_turns = 3;               
int CLICKS_PER_TURN = 1000; 
long fullyClosedPosition = n_turns * CLICKS_PER_TURN;

// The Interrupt Function for the Encoder
void IRAM_ATTR updateEncoder() {
  if (digitalRead(ENCODER_C2) == HIGH) {
    encoderCount++;
  } else {
    encoderCount--; 
  }
}

// ---------------------------------------------------------
// V0: Servo 1 (Joystick -100 to 100)
// ---------------------------------------------------------
BLYNK_WRITE(V0) {
  int throttle = param.asInt(); 
  int pwmSignal = map(throttle, -100, 100, 78, 102);
  Serial.println(pwmSignal);
  servo1.write(pwmSignal);
}
// ---------------------------------------------------------
// V1: Servo 2 (Joystick -100 to 100) - ASYMMETRIC MAPPING
// ---------------------------------------------------------
BLYNK_WRITE(V1) {
  int throttle = param.asInt(); 
  int pwmSignal;
  
  if (throttle < 0) {
    // LOWER HALF: -100 to -1 maps to 90 to 94
    pwmSignal = map(throttle, -100, -1, 90, 94); 
  } 
  else if (throttle > 0) {
    // UPPER HALF: 1 to 100 maps to 96 to 115
    pwmSignal = map(throttle, 1, 100, 96, 115); 
  } 
  else {
    // DEAD CENTER: Joystick is resting at exactly 0
    pwmSignal = 95; 
  }
  
  Serial.print("Joystick: ");
  Serial.print(throttle);
  Serial.print(" -> Asymmetric PWM: ");
  Serial.println(pwmSignal);
  
  servo2.write(pwmSignal);
}

// ---------------------------------------------------------
// V2: N20 Gripper (Switch 0 or 1)
// ---------------------------------------------------------
BLYNK_WRITE(V2) {
  int switchState = param.asInt(); 
  
  if (switchState == 100) {
    targetPosition = fullyClosedPosition;
    Serial.println("Closing Gripper to 3000...");
  } else if (switchState == 0) {
    targetPosition = 0;
    Serial.println("Opening Gripper to 0...");
  }
  else {
    Serial.println("Errors");
  }
}

void setup() {
  Serial.begin(115200);

  // --- SERVO SETUP ---
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servo1.setPeriodHertz(50); 
  servo2.setPeriodHertz(50); 
  servo1.attach(servo1Pin, 500, 2500); 
  servo2.attach(servo2Pin, 500, 2500); 
  
  // Force servos to a dead stop on startup
  servo2.write(neutralPoint);

  // --- N20 SETUP ---
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(ENCODER_C1, INPUT_PULLUP);
  pinMode(ENCODER_C2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_C1), updateEncoder, RISING);

  // Reset gripper position
  encoderCount = 0; 
  targetPosition = 0;

  Serial.println("Connecting to Wi-Fi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("System Online! Awaiting Dashboard Commands.");
}

void loop() {
  Blynk.run(); 

  // --- CONTINUOUS CLOSED-LOOP GRIPPER CONTROL ---
  long error = targetPosition - encoderCount;
  
  // Tolerance: How close is "good enough" to stop the motor?
  int tolerance = 25; 
  int motorSpeed = 230; 

  if (abs(error) <= tolerance) {
    // Target reached. Lock the brakes!
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, 255); // Maximum braking force
    
  } else if (error > tolerance) {
    // Target is ahead: Spin forward
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, motorSpeed);
    
  } else if (error < -tolerance) {
    // Target is behind: Spin reverse
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, motorSpeed);
  }
  
  delay(10); 
}
// Contributed to the soft gripper development branch