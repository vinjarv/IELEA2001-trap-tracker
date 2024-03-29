# Trap tracker
This project monitors the status of fishing traps like lobster or crab pots. Data is collected about water conditions, and images are taken to monitor how full the trap is.

We developed this for a project in the course IELEA2001 - "Computer Networks" at NTNU, autumn 2021. Our task was to develop an IoT-application where data is logged to a cloud service.


## Trap
Module that will be attached to the trap, submerged to ~15m
- Microcontroller: ESP32 devkit, any should work
- GPS-module: Adafruit Ultimate GPS Breakout

## Buoy
Module that is attached to the marking buoy/pulling rope

Add WiFi SSID, password and Ubidots device token to the file Code/Buoy/credentials.h
- Microcontroller: Ai-Thinker ESP32-CAM
- Temperature sensor: DS18B20

## Libraries
This project uses the following libraries in addition to the ESP32 board package:
- DallasTemperature - https://www.arduino.cc/reference/en/libraries/dallastemperature/
- Adafruit_GPS - https://github.com/adafruit/Adafruit_GPS/


## Ubidots widget
The HTML/CSS/JS files can be placed in a "HTML Widget" in a Ubidots dashboard to view the most recent image that has been collected.

Add jquery to "3rd party libraries":  https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js

Guide to using the HTML Widget:
https://help.ubidots.com/en/articles/754634-html-canvas-widget-examples
