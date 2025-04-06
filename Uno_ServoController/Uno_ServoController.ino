//********************************************************************
// ESP32 Code for Vision Controlled Spelling Board (GitHub Repo - Plan A Concept)
// Target: ESP32-CAM (AI Thinker Model)
// Function: Initializes camera, connects to WiFi/Blynk, SIMULATES pupil
//           tracking, sends servo commands to Arduino Uno via Serial2,
//           and sends resulting text to Blynk app.
//********************************************************************

#define BLYNK_PRINT Serial // Route Blynk library debug output to USB Serial
// --- UPDATED BLYNK DETAILS ---
#define BLYNK_TEMPLATE_ID "TMPL32qEpHOv9"
#define BLYNK_TEMPLATE_NAME "eternals"
#define BLYNK_AUTH_TOKEN "Yor3a0ah-mwmKj2maXXnplOP3fQOMgC3"
// --- END OF UPDATED BLYNK DETAILS ---

#include "esp_camera.h"       // Camera library
#include <WiFi.h>             // WiFi library
#include <WiFiClient.h>       // WiFi client library
#include <BlynkSimpleEsp32.h> // Blynk library

// --- Camera Model ---
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// --- WiFi Credentials --- (Make sure these are still correct for the network you'll use)
char ssid[] = "Galaxy A15 5G 17F7";
char pass[] = "pratyush.das123";

// --- Serial Communication with Arduino Uno ---
// Using Serial2 (GPIO17=TX2, GPIO16=RX2) - VERIFY THESE PINS ARE FREE ON YOUR BOARD!
HardwareSerial& SerialUno = Serial2;
const long UNO_BAUD_RATE = 9600;

// --- Blynk Objects & Timers ---
BlynkTimer timer;
const int BLYNK_TEXT_OUTPUT_PIN = V1; // Assumed Virtual pin for displaying text

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
  Serial.print("Template ID: "); Serial.println(BLYNK_TEMPLATE_ID); // Print details for verification
  Serial.print("Template Name: "); Serial.println(BLYNK_TEMPLATE_NAME);

  // --- Initialize Serial for Uno Communication ---
  // Args: baud, config, rxPin, txPin
  SerialUno.begin(UNO_BAUD_RATE, SERIAL_8N1, 16, 17); // RX=16, TX=17 for Serial2
  Serial.println("Serial2 for Uno communication started (TX=17, RX=16).");

  // --- Initialize Camera --- (Good to show camera init in conceptual code)
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
  Serial.printf("Connecting to WiFi: %s ", ssid);
  WiFi.begin(ssid, pass);
  int wifi_retries = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_retries < 20) { delay(500); Serial.print("."); wifi_retries++;}
  if(WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi Connection Failed! Check Credentials.");
  } else {
       Serial.println("\nWiFi connected");
  }
  Serial.print("Connecting to Blynk server...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
   wifi_retries = 0; // Reuse counter
   while (Blynk.connected() == false && wifi_retries < 20) { // Wait ~10 seconds
       delay(500); Serial.print("+");