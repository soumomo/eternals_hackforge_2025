//********************************************************************
// Arduino Uno Code for Vision Controlled Spelling Board (GitHub Repo - Plan A Concept)
// Target: Arduino Uno - CORRECTED VERSION - NO LEFT/RIGHT
// Function: Listens for commands ("UP", "DOWN", "CENTER", "SELECT", "RESET")
//           from ESP32 via Serial port and moves a Servo motor on Pin 9 accordingly.
//********************************************************************

#include <Servo.h>

// --- Pin Definitions ---
const int servoPin = 9; // Servo signal connected here

// --- Servo Object & Variables ---
Servo myServo;
int currentAngle = 90; // Current servo angle
// Define target angles for different commands (TUNE these for your physical layout)
const int centerAngle = 90;
const int upAngle = 60;    // Example angle for UP (adjust)
const int downAngle = 120; // Example angle for DOWN (adjust)
// SELECT might not move servo, or could wiggle

// --- Serial Communication ---
const long ESP32_BAUD_RATE = 9600; // Must match ESP32's SerialUno speed

//====================================================================
// SETUP
//====================================================================
void setup() {
  Serial.begin(ESP32_BAUD_RATE); // Start Serial for ESP32 communication
  Serial.println("Uno Servo Controller Ready (Plan A - No Left/Right). Waiting...");

  myServo.attach(servoPin);
  myServo.write(centerAngle); // Start servo at center position
  currentAngle = centerAngle;
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

    int targetAngle = currentAngle; // Default to current angle

    // Determine target angle based on command (No LEFT/RIGHT)
    if (command == "UP") {
      targetAngle = upAngle;
    } else if (command == "DOWN") {
      targetAngle = downAngle;
    } else if (command == "CENTER" || command == "RESET") {
      targetAngle = centerAngle;
    } else if (command == "SELECT") {
      // Optional: Do something on SELECT? Wiggle servo slightly? For now, do nothing.
      targetAngle = currentAngle; // Stay put
      Serial.println("SELECT command received (no movement).");
    } else {
      // Unknown command
      targetAngle = currentAngle; // Stay put
    }

    // Move servo only if target angle is valid and different
    targetAngle = constrain(targetAngle, 0, 180); // Ensure angle is valid

    if (targetAngle != currentAngle) {
      //Serial.print("Moving servo to: "); Serial.println(targetAngle); // Reduce spam maybe
      myServo.write(targetAngle);
      currentAngle = targetAngle;
    }
  }
} // End of loop()