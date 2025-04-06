# Project Eternals: Vision Controlled Spelling Board

## Hackathon Project - Hackforge - April 2025

**Team:** eternals
**Members:**
* Soumodeep Karmakar (Leader)
* Pratyush Das
* Souradip Daw

## Problem Statement

This project provides an accessible communication method for individuals with severe neuromuscular disorders (like ALS) or injuries that prevent speech and movement, addressing the significant communication barriers and isolation they face.

## Solution Implemented

We developed the "Vision Controlled Spelling Board," a system enabling users to communicate effectively using only eye movements. The system architecture utilizes:

1.  **Input & Processing:** An ESP32-CAM module performs real-time pupil tracking using its camera. It analyzes the captured video feed to detect gaze direction (UP, DOWN) and intentional blinks (used for selection). System activation and reset are controlled via the Blynk mobile application.
2.  **Control Communication:** The ESP32 processes the detected eye movements and sends corresponding commands ("UP", "DOWN", "SELECT", "RESET") via Serial communication to an Arduino Uno.
3.  **Physical Pointing:** The Arduino Uno receives these commands and precisely controls a Servo motor, which physically points to characters or words on a predefined layout, mirroring the user's gaze selection.
4.  **Output Display:** The ESP32 sends the selected character or word via WiFi directly to the Blynk application, displaying the generated text on the user's smartphone in real-time.

## Repository Contents

* `ESP32_PupilTracker_BlynkSender.ino` (within its folder): Code for the ESP32-CAM handling camera input, pupil tracking analysis, Blynk communication (input/output), and Serial commands to the Arduino. Requires Blynk Auth Token.
* `Uno_ServoController.ino` (within its folder): Code for the Arduino Uno, receiving commands via Serial from the ESP32 and controlling the Servo motor output.
* `README.md`: This file, explaining the project.
