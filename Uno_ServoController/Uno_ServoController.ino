#include <Servo.h> // Include the Servo library

// Define modes and commands (same as before)
String str1 = "UP";
String str2 = "DOWN";
String str3 = "BLINK"; // Confirmation command
String mode1 = "LETTER MODE";
String mode2 = "NUMBER MODE";
String mode3 = "KEYWORD MODE";
String rst = "RESET"; // Assuming RESET command might be needed

// Define colors (same as before)
String c1 = "YELLOW";
String c2 = "BLUE";
String c3 = "RED";
String c4 = "BLACK";
String c5 = "PINK";
String c6 = "GREEN";

// --- Servo Setup ---
#define SERVO_PIN 9 // Use a PWM pin for the Servo (e.g., pin 9 on Uno)
Servo myServo;      // Create a servo object

// --- Servo Angle Definitions ---
// !!! YOU MUST ADJUST THESE ANGLES (0-180) TO MATCH YOUR PHYSICAL BOARD LAYOUT !!!
const int servoAngles1[] = { 10,  40,  70, 100, 130, 160}; // 6 angles for colour/letter selection
const int POS1_COUNT = sizeof(servoAngles1) / sizeof(servoAngles1[0]);

const int servoAngles2[] = { 30, 150}; // 2 angles for number selection
const int POS2_COUNT = sizeof(servoAngles2) / sizeof(servoAngles2[0]);

const int servoAngles3[] = { 60, 120}; // 2 angles for keyword selection ("YES", "NO")
const int POS3_COUNT = sizeof(servoAngles3) / sizeof(servoAngles3[0]);

// --- State Variables (mostly same as before) ---
enum Mode { NONE, LETTER, NUMBER, KEYWORD };
Mode currentMode = NONE;
enum SelectionState { IDLE, SELECTING_COLOR, SELECTING_ITEM };
SelectionState currentState = IDLE;

int currentPositionIndex1 = 0; // Index for servoAngles1 array
int currentPositionIndex2 = 0; // Index for servoAngles2 array
int currentPositionIndex3 = 0; // Index for servoAngles3 array
String selectedColour = "";
char selectedLetter = ' ';
int selectedNumber = -1;
String selectedKeyword = "";
char finalSelectionChar = ' ';
int finalSelectionInt = -1;
String finalSelectionStr = "";

String inputStr; // Global variable to hold input string within loop iteration

//====================================================================
// SETUP FUNCTION
//====================================================================
void setup() {
  Serial.begin(9600); // Start Serial communication (ensure ESP32 uses same baud rate for Serial2)
  myServo.attach(SERVO_PIN); // Attach the servo object to the defined pin
  myServo.write(90);         // Move servo to center position initially (optional)
  delay(500);                // Wait for servo to settle
  Serial.println("Servo Control Ready. Send commands.");
  // No stepper speed/accel needed
}

//====================================================================
// MAIN LOOP
//====================================================================
void loop() {
  if (Serial.available() > 0) {
    inputStr = Serial.readStringUntil('\n'); // Read command from Serial
    inputStr.trim();                         // Remove whitespace

    Serial.print("Received: "); // Echo command for debugging
    Serial.println(inputStr);

    // --- Mode Handling ---
    if (inputStr.equalsIgnoreCase(mode1)) {
      currentMode = LETTER;
      currentState = SELECTING_COLOR;
      resetToHomePosition(); // Reset index and move servo to first position
      Serial.println("Mode changed to LETTER. Select Color.");
      myServo.write(servoAngles1[currentPositionIndex1]); // Go to first color position
      delay(300); // Allow servo to move
    } else if (inputStr.equalsIgnoreCase(mode2)) {
      currentMode = NUMBER;
      currentState = SELECTING_COLOR;
      resetToHomePosition();
      Serial.println("Mode changed to NUMBER. Select Color.");
      myServo.write(servoAngles1[currentPositionIndex1]); // Go to first color position
      delay(300);
    } else if (inputStr.equalsIgnoreCase(mode3)) {
      currentMode = KEYWORD;
      currentState = SELECTING_ITEM; // Direct selection for keyword
      resetToHomePosition();
      Serial.println("Mode changed to KEYWORD. Select YES/NO.");
      myServo.write(servoAngles3[currentPositionIndex3]); // Go to first keyword position
      delay(300);
    } else if (inputStr.equalsIgnoreCase(rst)) { // Handle RESET command
       resetToHomePosition();
       currentMode = NONE; // Go back to no mode
       currentState = IDLE;
       Serial.println("State Reset. Select a Mode.");
    }
    // --- Action Handling ---
    else if (currentMode != NONE) {
      if (inputStr.equalsIgnoreCase(str1)) { // UP
        handleUp();
      } else if (inputStr.equalsIgnoreCase(str2)) { // DOWN
        handleDown();
      } else if (inputStr.equalsIgnoreCase(str3)) { // BLINK (Select)
        handleBlink();
      }
    } else {
      Serial.println("No mode selected. Send LETTER MODE, NUMBER MODE, or KEYWORD MODE first.");
    }
  }
  // No blocking actions needed in loop for servo library usually
}

//====================================================================
// Command Handler Functions (Modified for Servo)
//====================================================================

void handleUp() {
  switch (currentMode) {
    case LETTER:
    case NUMBER:
      if (currentState == SELECTING_COLOR) {
        currentPositionIndex1++;
        if (currentPositionIndex1 >= POS1_COUNT) currentPositionIndex1 = 0; // Wrap
        myServo.write(servoAngles1[currentPositionIndex1]);
        delay(150); // Short delay for servo movement
      } else if (currentState == SELECTING_ITEM) {
        if (currentMode == LETTER) {
          currentPositionIndex1++;
          if (currentPositionIndex1 >= POS1_COUNT) currentPositionIndex1 = 0; // Wrap
          myServo.write(servoAngles1[currentPositionIndex1]);
          delay(150);
        } else { // NUMBER
          currentPositionIndex2++;
          if (currentPositionIndex2 >= POS2_COUNT) currentPositionIndex2 = 0; // Wrap
          myServo.write(servoAngles2[currentPositionIndex2]);
          delay(150);
        }
      }
      break;
    case KEYWORD:
      currentPositionIndex3++;
      if (currentPositionIndex3 >= POS3_COUNT) currentPositionIndex3 = 0; // Wrap
      myServo.write(servoAngles3[currentPositionIndex3]);
      delay(150);
      break;
    case NONE: break;
  }
    Serial.print("Current Index (1/2/3): "); Serial.print(currentPositionIndex1); Serial.print("/"); Serial.print(currentPositionIndex2); Serial.print("/"); Serial.println(currentPositionIndex3); // Debug
}

void handleDown() {
 switch (currentMode) {
    case LETTER:
    case NUMBER:
      if (currentState == SELECTING_COLOR) {
        currentPositionIndex1--;
        if (currentPositionIndex1 < 0) currentPositionIndex1 = POS1_COUNT - 1; // Wrap
        myServo.write(servoAngles1[currentPositionIndex1]);
        delay(150);
      } else if (currentState == SELECTING_ITEM) {
        if (currentMode == LETTER) {
          currentPositionIndex1--;
          if (currentPositionIndex1 < 0) currentPositionIndex1 = POS1_COUNT - 1; // Wrap
          myServo.write(servoAngles1[currentPositionIndex1]);
          delay(150);
        } else { // NUMBER
          currentPositionIndex2--;
          if (currentPositionIndex2 < 0) currentPositionIndex2 = POS2_COUNT - 1; // Wrap
          myServo.write(servoAngles2[currentPositionIndex2]);
          delay(150);
        }
      }
      break;
    case KEYWORD:
      currentPositionIndex3--;
      if (currentPositionIndex3 < 0) currentPositionIndex3 = POS3_COUNT - 1; // Wrap
      myServo.write(servoAngles3[currentPositionIndex3]);
      delay(150);
      break;
    case NONE: break;
  }
   Serial.print("Current Index (1/2/3): "); Serial.print(currentPositionIndex1); Serial.print("/"); Serial.print(currentPositionIndex2); Serial.print("/"); Serial.println(currentPositionIndex3); // Debug
}

void handleBlink() { // Selection Confirmation
  switch (currentMode) {
    case LETTER:
    case NUMBER:
      if (currentState == SELECTING_COLOR) {
        // Color selected based on currentPositionIndex1
        if (currentPositionIndex1 == 0) selectedColour = c1;
        else if (currentPositionIndex1 == 1) selectedColour = c2;
        else if (currentPositionIndex1 == 2) selectedColour = c3;
        else if (currentPositionIndex1 == 3) selectedColour = c4;
        else if (currentPositionIndex1 == 4) selectedColour = c5;
        else if (currentPositionIndex1 == 5) selectedColour = c6;
        else selectedColour = "";

        Serial.print("Color Selected: "); Serial.println(selectedColour);

        // Now ready for item selection, reset relevant index and move servo to start
        currentState = SELECTING_ITEM;
        if (currentMode == LETTER) {
             currentPositionIndex1 = 0; // Reset index for letter selection
             myServo.write(servoAngles1[currentPositionIndex1]); // Go to first letter position
             delay(300);
             Serial.println("Select Letter.");
        } else { // NUMBER
             currentPositionIndex2 = 0; // Reset index for number selection
             myServo.write(servoAngles2[currentPositionIndex2]); // Go to first number position
             delay(300);
              Serial.println("Select Number.");
        }

      } else if (currentState == SELECTING_ITEM) {
        // Final item selected based on color and current item index
        determineFinalSelection();
        printFinalSelection();
        // Return to idle state within this mode, ready for next color selection
        currentState = SELECTING_COLOR;
        currentPositionIndex1 = 0; // Reset color index
        myServo.write(servoAngles1[currentPositionIndex1]); // Go back to first color position
        delay(300);
        Serial.println("Selection complete. Select Color for next item or change mode.");
      }
      break;

    case KEYWORD:
       // Keyword selected based on currentPositionIndex3
       determineFinalSelection(); // Sets selectedKeyword
       printFinalSelection();
       // Stay in keyword mode, maybe reset index? Or require mode change command?
       // Let's reset index for simplicity
       currentPositionIndex3 = 0;
       myServo.write(servoAngles3[currentPositionIndex3]);
       delay(300);
       Serial.println("Selection complete. Select YES/NO again or change mode.");
      break;

    case NONE:
       Serial.println("Cannot select, no mode active.");
      break;
  }
}

// Determine final selection based on state variables
void determineFinalSelection() {
  // Reset selections
  selectedLetter = ' ';
  selectedNumber = -1;
  selectedKeyword = "";
  finalSelectionChar = ' ';
  finalSelectionInt = -1;
  finalSelectionStr = "";

  if (currentMode == LETTER) {
      // ... (LETTER Selection Logic identical to previous code) ...
      if (selectedColour == c1) { // YELLOW
        if (currentPositionIndex1 == 1) selectedLetter = 'H'; else if (currentPositionIndex1 == 2) selectedLetter = 'T'; else if (currentPositionIndex1 == 4) selectedLetter = 'N'; else if (currentPositionIndex1 == 5) selectedLetter = 'B';
      } else if (selectedColour == c2) { // BLUE
        if (currentPositionIndex1 == 1) selectedLetter = 'I'; else if (currentPositionIndex1 == 2) selectedLetter = 'U'; else if (currentPositionIndex1 == 4) selectedLetter = 'O'; else if (currentPositionIndex1 == 5) selectedLetter = 'C';
      } else if (selectedColour == c3) { // RED
          if (currentPositionIndex1 == 1) selectedLetter = 'L'; else if (currentPositionIndex1 == 2) selectedLetter = 'X'; else if (currentPositionIndex1 == 3) selectedLetter = 'Z'; else if (currentPositionIndex1 == 4) selectedLetter = 'R'; else if (currentPositionIndex1 == 5) selectedLetter = 'F';
      } else if (selectedColour == c4) { // BLACK
          if (currentPositionIndex1 == 1) selectedLetter = 'K'; else if (currentPositionIndex1 == 2) selectedLetter = 'W'; else if (currentPositionIndex1 == 3) selectedLetter = 'Y'; else if (currentPositionIndex1 == 4) selectedLetter = 'Q'; else if (currentPositionIndex1 == 5) selectedLetter = 'E';
      } else if (selectedColour == c5) { // PINK
          if (currentPositionIndex1 == 1) selectedLetter = 'J'; else if (currentPositionIndex1 == 2) selectedLetter = 'V'; else if (currentPositionIndex1 == 4) selectedLetter = 'P'; else if (currentPositionIndex1 == 5) selectedLetter = 'D';
      } else if (selectedColour == c6) { // GREEN
          if (currentPositionIndex1 == 1) selectedLetter = 'G'; else if (currentPositionIndex1 == 2) selectedLetter = 'S'; else if (currentPositionIndex1 == 4) selectedLetter = 'M'; else if (currentPositionIndex1 == 5) selectedLetter = 'A';
      }
      finalSelectionChar = selectedLetter;

  } else if (currentMode == NUMBER) {
      // ... (NUMBER Selection Logic identical to previous code) ...
       if (selectedColour == c1) { // YELLOW
          if (currentPositionIndex2 == 0) selectedNumber = 2; else if (currentPositionIndex2 == 1) selectedNumber = 8;
      } else if (selectedColour == c2) { // BLUE
          if (currentPositionIndex2 == 0) selectedNumber = 3; else if (currentPositionIndex2 == 1) selectedNumber = 9;
      } else if (selectedColour == c3) { // RED
          if (currentPositionIndex2 == 0) selectedNumber = 6;
      } else if (selectedColour == c4) { // BLACK
          if (currentPositionIndex2 == 0) selectedNumber = 5;
      } else if (selectedColour == c5) { // PINK
          if (currentPositionIndex2 == 0) selectedNumber = 4; else if (currentPositionIndex2 == 1) selectedNumber = 0;
      } else if (selectedColour == c6) { // GREEN
          if (currentPositionIndex2 == 0) selectedNumber = 1; else if (currentPositionIndex2 == 1) selectedNumber = 7;
      }
       finalSelectionInt = selectedNumber;

  } else if (currentMode == KEYWORD) {
       if (currentPositionIndex3 == 0) selectedKeyword = "YES";
       else if (currentPositionIndex3 == 1) selectedKeyword = "NO";
       else selectedKeyword = "";
       finalSelectionStr = selectedKeyword;
  }
}

// Print final selection to Serial Monitor
void printFinalSelection() {
  Serial.print("Final Selection: ");
  if (finalSelectionChar != ' ') {
    Serial.println(finalSelectionChar);
    // If ESP32 needs the final char, send it back here via Serial TX
    // Serial.print("FINAL:"); Serial.println(finalSelectionChar);
  } else if (finalSelectionInt != -1) {
    Serial.println(finalSelectionInt);
    // Serial.print("FINAL:"); Serial.println(finalSelectionInt);
  } else if (finalSelectionStr != "") {
     Serial.println(finalSelectionStr);
     // Serial.print("FINAL:"); Serial.println(finalSelectionStr);
  } else {
    Serial.println("[Nothing Selected]");
  }
}

// --- Utility Functions ---

// Reset indices and move servo to first position of current mode
void resetToHomePosition() {
  currentPositionIndex1 = 0;
  currentPositionIndex2 = 0;
  currentPositionIndex3 = 0;
  selectedColour = "";
  selectedLetter = ' ';
  selectedNumber = -1;
  selectedKeyword = "";
  finalSelectionChar = ' ';
  finalSelectionInt = -1;
  finalSelectionStr = "";

  // Move servo to initial position based on mode
  if (currentMode == LETTER || currentMode == NUMBER) {
      currentState = SELECTING_COLOR;
      // We don't move servo here, assume mode change command handles initial move
  } else if (currentMode == KEYWORD) {
      currentState = SELECTING_ITEM;
      // We don't move servo here, assume mode change command handles initial move
  } else {
      currentState = IDLE;
      myServo.write(servoAngles1[0]); // Go to absolute 0 angle if no mode? Or center (90)?
      delay(300);
  }
}
