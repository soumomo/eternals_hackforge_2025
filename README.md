# Project Eternals: Vision Controlled Spelling Board

## Hackathon Project - Hackforge - April 2025

**Team:** eternals
**Members:**
* Soumodeep Karmakar (Leader)
* Pratyush Das
* Souradip Daw

## Problem Statement

This project aims to provide an accessible communication method for individuals with severe neuromuscular disorders (like ALS) or injuries that prevent speech and movement, who often face significant communication barriers and isolation.

## Proposed Solution (Ideal Architecture - Plan A)

We propose the "Vision Controlled Spelling Board," a system enabling users to communicate using only eye movements. The intended architecture is:

1.  **Input:** An ESP32-CAM module performs pupil tracking (detecting gaze direction UP/DOWN/LEFT/RIGHT and BLINKS for selection) using its camera. *(Note: Due to hackathon constraints like lack of ideal IR illumination for robust pupil tracking with this specific hardware, the tracking logic in the ESP32 code is currently simulated/placeholder but demonstrates the intended data flow).*
2.  **Control:** The ESP32 sends simple commands based on the detected eye movements/blinks via Serial to an Arduino Uno.
3.  **Pointing:** The Arduino Uno controls a Servo motor to physically point to corresponding characters/words on a predefined layout based on the received commands.
4.  **Output:** The ESP32 simultaneously sends the selected character/word/text via WiFi to a Blynk application for display on a mobile device, providing text-to-speech potential.

## Repository Contents

* `ESP32_PupilTracker_BlynkSender.ino` (within its folder): Conceptual code for the ESP32-CAM implementing the core logic flow with **simulated** pupil tracking and Blynk output. Requires Blynk Auth Token.
* `Uno_ServoController.ino` (within its folder): Conceptual code for the Arduino Uno, receiving commands via Serial and controlling the Servo motor.
* `README.md`: This file, explaining the project's intended vision.
