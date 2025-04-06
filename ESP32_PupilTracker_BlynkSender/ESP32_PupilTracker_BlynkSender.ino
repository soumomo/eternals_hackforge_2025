//********************************************************************
// ESP32 Code for Vision Controlled Spelling Board (GitHub Repo - Plan A Concept)
// Target: ESP32-CAM (AI Thinker Model)
// Function: Initializes camera, connects to WiFi/Blynk, SIMULATES pupil
//           tracking, sends servo commands to Arduino Uno via Serial2,
//           and sends resulting text to Blynk app.
//********************************************************************

#define BLYNK_PRINT Serial // Route Blynk library debug output to USB Serial
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"    // *** PASTE YOUR BLYNK TEMPLATE ID HERE ***
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"// *** PASTE YOUR BLYNK TEMPLATE NAME HERE ***
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"        // *** PASTE YOUR BLYNK AUTH TOKEN HERE ***

#include "esp_camera.h"       // Camera library
#include <WiFi.h>             // WiFi library
#include <WiFiClient.h>       // WiFi client library
#include <BlynkSimpleEsp32.h> // Blynk library

// --- Camera Model ---
#define CAMERA_MODEL_AI_THINKER
#include "pratyush.das123"

// --- WiFi Credentials ---
char ssid[] = "Galaxy A15 5G 17F7"; // Your WiFi Network Name
char pass[] = "44953B691C60"; // Your WiFi Password

// --- Serial Communication with Arduino Uno ---
// Using Serial2 (GPIO17=TX2, GPIO16=RX2) - VERIFY THESE PINS ARE FREE ON YOUR BOARD!
HardwareSerial& SerialUno = Serial2;
const long UNO_BAUD_RATE = 9600;

// --- Blynk Objects & Timers ---
BlynkTimer timer;
const int BLYNK_TEXT_OUTPUT_PIN = V1; // Virtual pin for displaying text in Blynk

// --- Simulation Variables ---
int sim_state_counter = 0;
unsigned long lastSimTime = 0;
const unsigned long SIM_INTERVAL = 3000; // Simulate a command every 3 seconds

//====================================================================
// SETUP
//====================================================================
void setup() {
  Serial.begin(115200); // USB Serial for Debugging
  Serial.setDebugOutput(true);
  Serial.println("\n\nESP32 Pupil Tracker Simulation (Plan A)");
  Serial.println("--------------------------------------");

  // --- Initialize Serial for Uno Communication ---
  // Args: baud, config, rxPin, txPin
  SerialUno.begin(UNO_BAUD_RATE, SERIAL_8N1, 16, 17); // RX=16, TX=17 for Serial2
  Serial.println("Serial2 for Uno communication started (TX=17, RX=16).");

  // --- Initialize Camera --- (Good to show camera init in conceptual code)
  camera_config_t config;
  // Populate config struct (same as previous full examples)
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM; config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.pixel_format = PIXFORMAT_GRAYSCALE; // Grayscale is fine for concept
  config.frame_size = FRAMESIZE_QQVGA;    // Low resolution for concept
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("!!! Conceptual Camera init failed: 0x%x (Ignoring for simulation) !!!\n", err);
  } else {
    Serial.println("Conceptual Camera Initialized Successfully");
  }

  // --- Connect to WiFi & Blynk ---
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, pass);
  int wifi_retries = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_retries < 20) { delay(500); Serial.print("."); wifi_retries++;}
  if(WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi Connection Failed! Check Credentials.");
      // Don't necessarily restart in conceptual code, Blynk connect will fail below
  } else {
       Serial.println("\nWiFi connected");
  }
  Serial.print("Connecting to Blynk...");
  // Note: Add timeout if needed: Blynk.config(BLYNK_AUTH_TOKEN, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT, timeout);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
   wifi_retries = 0; // Reuse counter for Blynk timeout
   while (Blynk.connected() == false && wifi_retries < 20) { // Wait ~10 seconds for Blynk
       delay(500);
       Serial.print("+");
       wifi_retries++;
   }
   if(Blynk.connected()){
       Serial.println(" Connected!");
   } else {
       Serial.println(" FAILED or timed out!");
   }


  Serial.println("Setup Complete. Running Pupil Tracking Simulation.");
} // End of setup()


//====================================================================
// MAIN LOOP
//====================================================================
void loop() {
  if(Blynk.connected()){ // Only run Blynk if connected
     Blynk.run();
  }
  timer.run(); // Run Blynk timers if any are set

  // *** SIMULATED PUPIL TRACKING LOGIC ***
  // This section simulates the output of a working pupil tracker.
  // It cycles through different commands every SIM_INTERVAL milliseconds.
  unsigned long currentTime = millis();
  if (currentTime - lastSimTime > SIM_INTERVAL) {

    String commandForUno = "";
    String textForBlynk = "";

    // Simulate cycling through states
    sim_state_counter++;
    if (sim_state_counter > 6) sim_state_counter = 0; // Added more states

    Serial.print("Simulating state: "); Serial.println(sim_state_counter);

    switch (sim_state_counter) {
      case 0: commandForUno = "CENTER"; textForBlynk = " "; break;     // Go to Center
      case 1: commandForUno = "RIGHT"; /* User looks right */ break;
      case 2: commandForUno = "SELECT"; textForBlynk = "E"; break;    // User selects (e.g., via blink), assume 'E' based on right pos
      case 3: commandForUno = "DOWN"; /* User looks down */ break;
      case 4: commandForUno = "SELECT"; textForBlynk = "S"; break;    // User selects, assume 'S' based on down pos
      case 5: commandForUno = "LEFT"; /* User looks left */ break;
      case 6: commandForUno = "RESET"; textForBlynk = ""; break;      // Reset position
    }

    // --- Send Command to Arduino Uno via Wired Serial (Serial2 TX = GPIO17) ---
    if (commandForUno.length() > 0) {
      Serial.printf("Simulating: Sending command '%s' to Uno...\n", commandForUno.c_str());
      SerialUno.println(commandForUno);
    }

    // --- Send Resulting Text to Blynk V1 ---
    if (textForBlynk.length() > 0) {
       Serial.printf("Simulating: Sending text '%s' to Blynk V%d...\n", textForBlynk.c_str(), BLYNK_TEXT_OUTPUT_PIN);
       if(Blynk.connected()) { // Only write if connected
          Blynk.virtualWrite(BLYNK_TEXT_OUTPUT_PIN, textForBlynk);
       } else {
          Serial.println("(Blynk disconnected, cannot send text)");
       }
    } else if (commandForUno == "RESET") {
       // Clear Blynk display on reset
        if(Blynk.connected()) {
          Blynk.virtualWrite(BLYNK_TEXT_OUTPUT_PIN, " ");
        }
    }

    lastSimTime = currentTime;
  }

  yield(); // Allow background tasks

} // End of loop()


// --- Optional Blynk Handlers ---
/*
BLYNK_CONNECTED() {
  // Optional: Request server sync when ESP32 connects to Blynk
  // Blynk.syncVirtual(BLYNK_TEXT_OUTPUT_PIN);
}

BLYNK_WRITE(V0) { // Example if you had an input widget on V0
  int value = param.asInt();
  Serial.printf("Received V0 value from Blynk: %d\n", value);
  // Add logic here if needed
}
*/