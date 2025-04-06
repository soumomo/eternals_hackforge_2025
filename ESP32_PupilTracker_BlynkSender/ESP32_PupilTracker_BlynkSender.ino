//********************************************************************
// ESP32 Code for Vision Controlled Spelling Board (GitHub Repo - Plan A Concept)
// Target: ESP32-CAM (AI Thinker Model) - FINAL GITHUB v3 - NO L/R/CENTER
// Function: Simulates pupil tracking (UP/DOWN/SELECT only), handles Blynk input
//           (V0 Mode, V2 Reset), prints state to Serial Monitor, sends servo commands
//           to Arduino Uno via Serial2, and sends resulting text to Blynk V1.
//********************************************************************

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL32qEpHOv9"
#define BLYNK_TEMPLATE_NAME "eternals"
#define BLYNK_AUTH_TOKEN "Yor3a0ah-mwmKj2maXXnplOP3fQOMgC3" // Use your valid token

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// --- Camera Model ---
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// --- WiFi Credentials ---
char ssid[] = "NARZO 70 Pro 5G";
char pass[] = "k3g6cju2";

// --- Serial Communication with Arduino Uno ---
HardwareSerial& SerialUno = Serial2; // RX=16, TX=17 - VERIFY PINS!
const long UNO_BAUD_RATE = 9600;

// --- Blynk Objects & Timers ---
BlynkTimer timer;
const int BLYNK_MODE_TOGGLE_PIN = V0;
const int BLYNK_TEXT_OUTPUT_PIN = V1;
const int BLYNK_RESET_PIN = V2;

// --- State & Simulation Variables ---
bool systemActive = false;
// Simplified states - removed CENTER, LEFT, RIGHT
enum SimGazeState { STATE_UP, STATE_DOWN, STATE_SELECT, STATE_RESET, STATE_INIT };
SimGazeState currentSimState = STATE_INIT;
unsigned long lastSimActionTime = 0;
const unsigned long SIM_ACTION_INTERVAL = 3500;
String currentLetterContext = " "; // What letter context corresponds to UP or DOWN state

//====================================================================
// SETUP
//====================================================================
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("\n\nESP32 Plan A Sim - No L/R/Center");
  Serial.println("---------------------------------");
  Serial.print("Blynk Template: "); Serial.print(BLYNK_TEMPLATE_NAME); Serial.print(" / ID: "); Serial.println(BLYNK_TEMPLATE_ID);

  SerialUno.begin(UNO_BAUD_RATE, SERIAL_8N1, 16, 17);
  Serial.println("Serial2 for Uno started (TX=17, RX=16). Baud: " + String(UNO_BAUD_RATE));

  // --- Conceptual Camera Initialization ---
  camera_config_t config;
  // (Populate config struct - same as previous: GRAYSCALE, QQVGA etc)
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
  if (err == ESP_OK) { Serial.println("Conceptual Camera Initialized"); }
  else { Serial.printf("!!! Conceptual Camera init failed: 0x%x !!!\n", err); }

  // --- Connect to WiFi & Blynk ---
  Serial.printf("Connecting to WiFi: %s ", ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("\nAttempting Blynk connection...");
  int timeout_counter = 0;
  while (Blynk.connected() == false && timeout_counter < 20) { delay(500); Serial.print("+"); timeout_counter++; }
  if(Blynk.connected()){ Serial.println(" Connected!"); }
  else { Serial.println(" FAILED or timed out!"); }

  Serial.println("Setup Complete. Waiting for Blynk V0 activation...");
} // End of setup()


// --- Blynk Input Handlers ---
BLYNK_WRITE(V0) { // Mode Toggle
  int value = param.asInt();
  if (value == 1) {
    systemActive = true;
    Serial.println("Received Mode ON from Blynk [V0]. Activating simulation.");
    SerialUno.println("MODE_ON");
    SerialUno.println("RESET"); // Tell Uno to go to initial position
    currentSimState = STATE_RESET; // Start simulation at reset state
    currentLetterContext = " ";
    if(Blynk.connected()) { Blynk.virtualWrite(BLYNK_TEXT_OUTPUT_PIN, " "); }
  } else {
    systemActive = false;
    Serial.println("Received Mode OFF from Blynk [V0]. Deactivating.");
    SerialUno.println("MODE_OFF");
  }
}

BLYNK_WRITE(V2) { // Reset Command from Blynk
  int value = param.asInt();
  if (value == 1 && systemActive) {
    Serial.println("Received RESET command from Blynk [V2]");
    Serial.println("-> Sending 'RESET' command to Uno...");
    SerialUno.println("RESET"); // Send RESET command to Uno
    Serial.println("-> Clearing text on Blynk V1...");
    if(Blynk.connected()) { Blynk.virtualWrite(BLYNK_TEXT_OUTPUT_PIN, " "); }
    currentSimState = STATE_RESET; // Reset internal state
    currentLetterContext = " ";
    lastSimActionTime = millis();
  }
}

//====================================================================
// MAIN LOOP
//====================================================================
void loop() {
  if(Blynk.connected()){ Blynk.run(); }
  timer.run();

  if (!systemActive) { yield(); return; } // Only run simulation if active

  // *** SIMULATED PUPIL TRACKING LOGIC (UP/DOWN/SELECT/RESET Only) ***
  unsigned long currentTime = millis();
  if (currentTime - lastSimActionTime > SIM_ACTION_INTERVAL) {

    String commandForUno = "";
    String textForBlynk = "";

    // Simulate cycling through states: RESET -> UP -> SELECT -> DOWN -> SELECT -> RESET -> ...
    SimGazeState nextState;
    switch(currentSimState) {
        case STATE_RESET:  nextState = STATE_UP; break;
        case STATE_UP:     nextState = STATE_SELECT; break;
        case STATE_DOWN:   nextState = STATE_SELECT; break;
        case STATE_SELECT: // After selecting, decide where to go next
                           nextState = (currentLetterContext == "U") ? STATE_DOWN : STATE_RESET; // Go DOWN after UP select, otherwise RESET
                           break;
        default:           nextState = STATE_RESET; break; // Default to RESET
    }
    currentSimState = nextState; // Update state for this cycle

    // Determine commands and text based on the *new* state
    switch (currentSimState) {
      case STATE_UP:     commandForUno = "UP"; textForBlynk = ""; currentLetterContext = "U"; break; // Context is 'U'
      case STATE_DOWN:   commandForUno = "DOWN"; textForBlynk = ""; currentLetterContext = "D"; break; // Context is 'D'
      case STATE_SELECT: // Select is triggered (simulated blink)
          commandForUno = "SELECT"; // Tell Uno to wiggle or indicate selection visually
          textForBlynk = currentLetterContext; // Send the letter from the previous context (U or D)
          break;
      case STATE_RESET:
          commandForUno = "RESET"; textForBlynk = ""; currentLetterContext = " "; break;
       // No CENTER case needed
    }

    String currentSimulatedStateString = stateToString(currentSimState); // Convert enum to string for print/send

    // --- Print Simulated State to Local Serial Monitor ---
    Serial.printf("Simulated State: %s\n", currentSimulatedStateString.c_str());

    // --- Send Command to Arduino Uno via Wired Serial (Serial2 TX = GPIO17) ---
    if (commandForUno.length() > 0) {
      Serial.printf("-> Sending command '%s' to Uno...\n", commandForUno.c_str());
      SerialUno.println(commandForUno);
    }

    // --- Send Resulting Text to Blynk V1 (Only on SELECT state) ---
    if (currentSimState == STATE_SELECT && textForBlynk.length() > 0) {
       Serial.printf("-> Sending text '%s' to Blynk V%d...\n", textForBlynk.c_str(), BLYNK_TEXT_OUTPUT_PIN);
       if(Blynk.connected()) {
          Blynk.virtualWrite(BLYNK_TEXT_OUTPUT_PIN, textForBlynk);
       } else {
          Serial.println("(Blynk disconnected)");
       }
    } else if (currentSimState == STATE_RESET) {
        // Clear Blynk display on reset state
         if(Blynk.connected()) { Blynk.virtualWrite(BLYNK_TEXT_OUTPUT_PIN, " "); }
    }

    lastSimActionTime = currentTime; // Reset timer for next simulation step
  }
  yield();
} // End of loop()

// --- Helper Function: State to String ---
String stateToString(SimGazeState state) {
  switch (state) {
    case STATE_UP:     return "UP";
    case STATE_DOWN:   return "DOWN";
    case STATE_SELECT: return "SELECT";
    case STATE_RESET:  return "RESET";
    default:           return "INIT"; // Or "UNKNOWN"
  }
}
