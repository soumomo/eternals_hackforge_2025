//********************************************************************
// ESP32 Code for Vision Controlled Spelling Board (GitHub Repo - Plan A Concept)
// Target: ESP32-CAM (AI Thinker Model) - CORRECTED VERSION - NO LEFT/RIGHT
// Function: Initializes camera, connects to WiFi/Blynk, SIMULATES pupil
//           tracking (UP/DOWN/BLINK/CENTER only), prints state to Serial Monitor,
//           sends servo commands to Arduino Uno via Serial2, and sends
//           resulting text to Blynk app. Listens for Mode (V0) and Reset (V2).
//********************************************************************

#define BLYNK_PRINT Serial // Route Blynk library debug output to USB Serial
// --- Blynk Credentials ---
#define BLYNK_TEMPLATE_ID "TMPL32qEpHOv9"
#define BLYNK_TEMPLATE_NAME "eternals"
#define BLYNK_AUTH_TOKEN "Yor3a0ah-mwmKj2maXXnplOP3fQOMgC3"

#include "esp_camera.h"       // Camera library
#include <WiFi.h>             // WiFi library
#include <WiFiClient.h>       // WiFi client library
#include <BlynkSimpleEsp32.h> // Blynk library

// --- Camera Model ---
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// --- WiFi Credentials ---
char ssid[] = "Galaxy A15 5G 17F7";
char pass[] = "pratyush.das123";

// --- Serial Communication with Arduino Uno ---
// Using Serial2 (GPIO17=TX2, GPIO16=RX2) - VERIFY THESE PINS ARE FREE!
HardwareSerial& SerialUno = Serial2;
const long UNO_BAUD_RATE = 9600;

// --- Blynk Objects & Timers ---
BlynkTimer timer;
const int BLYNK_MODE_TOGGLE_PIN = V0; // Virtual pin for Mode toggle button
const int BLYNK_TEXT_OUTPUT_PIN = V1; // Virtual pin for displaying text in Blynk
const int BLYNK_RESET_PIN = V2;       // Virtual pin for Reset button

// --- State & Simulation Variables ---
bool textModeActive = false; // Controlled by Blynk V0
int sim_state_counter = 0;
unsigned long lastSimTime = 0;
const unsigned long SIM_INTERVAL = 3000; // Simulate a command every 3 seconds
// Note: Simplified states - removed LEFT/RIGHT
enum SimState { STATE_CENTER, STATE_UP, STATE_DOWN, STATE_SELECT, STATE_RESET };
SimState currentSimulatedStateEnum = STATE_CENTER;
String currentSimulatedStateString = "CENTER"; // For sending/displaying

//====================================================================
// SETUP
//====================================================================
void setup() {
  Serial.begin(115200); // USB Serial for Debugging
  Serial.setDebugOutput(true);
  Serial.println("\n\nESP32 Pupil Tracker Simulation (Plan A - No Left/Right)");
  Serial.println("------------------------------------------------------");
  Serial.print("Blynk Template: "); Serial.print(BLYNK_TEMPLATE_NAME);
  Serial.print(" / ID: "); Serial.println(BLYNK_TEMPLATE_ID);

  // --- Initialize Serial for Uno Communication ---
  SerialUno.begin(UNO_BAUD_RATE, SERIAL_8N1, 16, 17); // RX=16, TX=17 for Serial2
  Serial.println("Serial2 for Uno communication started (TX=17, RX=16).");

  // --- Initialize Camera --- (Conceptual)
  camera_config_t config;
  // (Full camera config struct population - same as previous version)
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM; config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM; config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_QQVGA; config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("!!! Conceptual Camera init failed: 0x%x (Ignoring for simulation) !!!\n", err);
  } else {
    Serial.println("Conceptual Camera Initialized Successfully");
  }

  // --- Connect to WiFi & Blynk ---
  Serial.printf("Connecting to WiFi: %s ", ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("\nAttempting Blynk connection...");
  int timeout_counter = 0;
  while (Blynk.connected() == false && timeout_counter < 20) { delay(500); Serial.print("+"); timeout_counter++; }
  if(Blynk.connected()){ Serial.println(" Connected!"); }
  else { Serial.println(" FAILED or timed out!"); }

  Serial.println("Setup Complete. Waiting for Blynk commands / Running Simulation.");
} // End of setup()


// --- Blynk Input Handlers ---
BLYNK_WRITE(V0) { // Mode Toggle
  int value = param.asInt();
  if (value == 1) {
    textModeActive = true;
    Serial.println("Received Text Mode ON from Blynk [V0]");
    SerialUno.println("MODE_ON"); // Tell Uno mode is ON (if needed)
  } else {
    textModeActive = false;
    Serial.println("Received Text Mode OFF from Blynk [V0]");
    SerialUno.println("MODE_OFF"); // Tell Uno mode is OFF
  }
}

BLYNK_WRITE(V2) { // Reset Command
  int value = param.asInt();
  if (value == 1) {
    Serial.println("Received RESET command from Blynk [V2]");
    currentSimulatedStateEnum = STATE_RESET; // Set state
    currentSimulatedStateString = stateToString(currentSimulatedStateEnum);
    Serial.printf("-> Sending '%s' to Uno...\n", currentSimulatedStateString.c_str());
    SerialUno.println(currentSimulatedStateString); // Send RESET command to Uno
    Serial.printf("-> Clearing text on Blynk V%d...\n", BLYNK_TEXT_OUTPUT_PIN);
    Blynk.virtualWrite(BLYNK_TEXT_OUTPUT_PIN, " "); // Clear Blynk display
    sim_state_counter = 0; // Reset simulation cycle
    lastSimTime = millis();
  }
}

//====================================================================
// MAIN LOOP
//====================================================================
void loop() {
  if(Blynk.connected()){ Blynk.run(); }
  timer.run();

  // Only run simulation if text mode is active
  if (!textModeActive) {
    yield(); return;
  }

  // *** SIMULATED PUPIL TRACKING LOGIC (UP/DOWN/SELECT/CENTER/RESET Only) ***
  unsigned long currentTime = millis();
  if (currentTime - lastSimTime > SIM_INTERVAL) {

    String commandForUno = "";
    String textForBlynk = "";

    // Simulate cycling through states (excluding Left/Right)
    sim_state_counter++;
    if (sim_state_counter > 4) sim_state_counter = 0; // Cycle through 0, 1, 2, 3, 4

    switch (sim_state_counter) {
      case 0: currentSimulatedStateEnum = STATE_CENTER; textForBlynk = ""; break;    // Center
      case 1: currentSimulatedStateEnum = STATE_UP; textForBlynk = ""; break;       // Look Up
      case 2: currentSimulatedStateEnum = STATE_SELECT; textForBlynk = "T"; break;  // Select 'T' (assume Up position)
      case 3: currentSimulatedStateEnum = STATE_DOWN; textForBlynk = ""; break;     // Look Down
      case 4: currentSimulatedStateEnum = STATE_SELECT; textForBlynk = "A"; break;  // Select 'A' (assume Down position)
      // Implicitly loops back to CENTER on next cycle after state 4
      // RESET state is triggered only by Blynk V2 now
    }
    currentSimulatedStateString = stateToString(currentSimulatedStateEnum);
    commandForUno = currentSimulatedStateString; // Command for Uno is the state name

    // --- Print Simulated State to Serial Monitor ---
    Serial.printf("Simulated State: %s\n", currentSimulatedStateString.c_str());

    // --- Send Command to Arduino Uno via Wired Serial (Serial2 TX = GPIO17) ---
    if (commandForUno.length() > 0) {
      Serial.printf("-> Sending command '%s' to Uno...\n", commandForUno.c_str());
      SerialUno.println(commandForUno);
    }

    // --- Send Resulting Text to Blynk V1 (Only on SELECT state) ---
    if (currentSimulatedStateEnum == STATE_SELECT && textForBlynk.length() > 0) {
       Serial.printf("-> Sending text '%s' to Blynk V%d...\n", textForBlynk.c_str(), BLYNK_TEXT_OUTPUT_PIN);
       if(Blynk.connected()) {
          Blynk.virtualWrite(BLYNK_TEXT_OUTPUT_PIN, textForBlynk);
       } else {
          Serial.println("(Blynk disconnected)");
       }
    }
    // Consider if RESET should clear Blynk display (handled in BLYNK_WRITE(V2) now)

    lastSimTime = currentTime;
  }
  yield();
} // End of loop()

// --- Helper Function: State to String ---
String stateToString(SimState state) {
  switch (state) {
    case STATE_CENTER: return "CENTER";
    case STATE_UP:     return "UP";
    case STATE_DOWN:   return "DOWN";
    case STATE_SELECT: return "SELECT"; // Changed from BLINK
    case STATE_RESET:  return "RESET";  // Added RESET state
    // case STATE_LOST: return "LOST"; // Removed if not needed in simulation
    default:           return "INIT";
  }
}