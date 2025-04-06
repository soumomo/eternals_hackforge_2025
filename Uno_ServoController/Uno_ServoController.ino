//********************************************************************
// Arduino Uno Code for Vision Controlled Spelling Board (GitHub Repo - Plan A Concept)
// Target: Arduino Uno - FINAL GITHUB VERSION - NO L/R/CENTER
// Function: Listens for commands ("UP", "DOWN", "SELECT", "RESET", "MODE_ON", "MODE_OFF")
//           from ESP32 via Serial port and moves a Servo motor on Pin 9 accordingly.
//********************************************************************

#include <Servo.h>

// --- Pin Definitions ---
const int servoPin = 9; // Servo signal connected here

// --- Servo Object & Variables ---
Servo myServo;
int currentAngle = 0; // Start at 0 (RESET position)
const int stepDegrees = 60; // Degrees to move for UP/DOWN (TUNE THIS)
const int maxAngle = 180;
const int minAngle = 0;

// --- Serial Communication ---
const long ESP32_BAUD_RATE = 9600; // Must match ESP32's SerialUno speed

// --- State ---
bool systemActive = false; // Track if ESP32 has activated the mode

//====================================================================
// SETUP
//====================================================================
void setup() {
  Serial.begin(ESP32_BAUD_RATE); // Start Serial for ESP32 communication
  Serial.println("Uno Servo Controller Ready (Plan A - No L/R/Center). Waiting for MODE_ON...");

  myServo.attach(servoPin);
  myServo.write(currentAngle); // Start servo at initial 0 position
  Serial.print("Servo initialized at: "); Serial.println(currentAngle);
} // End of setup()

//====================================================================
// MAIN LOOP
//====================================================================
void loop() {
  // Check for commands from ESP32
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    Serial.print("Received Command: "); // Echo for debugging Uno side
    Serial.println(command);

    // Update active state first
    if (command == "MODE_ON") {
      systemActive = true;
      Serial.println("System Activated by ESP32.");
      // Ensure servo is at reset position when activated
      currentAngle = 0;
      myServo.write(currentAngle);
      return; // Don't process MODE_ON as a movement
    } else if (command == "MODE_OFF") {
      systemActive = false;
      Serial.println("System Deactivated by ESP32.");
      // Optional: Move servo to resting position?
      // currentAngle = 0;
      // myServo.write(currentAngle);
      return; // Don't process MODE_OFF as a movement
    }

    // Only process movement commands if system is active
    if (!systemActive) {
      return;
    }

    int targetAngle = currentAngle; // Default to current angle

    // Determine target angle based on command (No LEFT/RIGHT/CENTER)
    if (command == "UP") {
      targetAngle = currentAngle + stepDegrees;
    } else if (command == "DOWN") {
      targetAngle = currentAngle - stepDegrees;
    } else if (command == "RESET") {
      targetAngle = 0; // Move to absolute zero
    } else if (command == "SELECT") {
      // Wiggle servo slightly for feedback on SELECT
      myServo.write(constrain(currentAngle - 5, minAngle, maxAngle));
      delay(100);
      myServo.write(constrain(currentAngle + 5, minAngle, maxAngle));
      delay(100);
      targetAngle = currentAngle; // Return to current position after wiggle
      Serial.println("SELECT command processed (wiggled).");
    } else {
      // Unknown movement command
      Serial.print("Unknown movement command: "); Serial.println(command);
      targetAngle = currentAngle; // Stay put
    }

    // Clamp target angle to limits (0-180 for typical servos)
    targetAngle = constrain(targetAngle, minAngle, maxAngle);

    // Move servo only if target angle is different
    if (targetAngle != currentAngle) {
      Serial.print("Moving servo to: "); Serial.println(targetAngle);
      myServo.write(targetAngle);
      currentAngle = targetAngle; // Update current angle
    }
  }
} // End of loop()