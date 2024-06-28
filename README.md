# Microcontroller-based-temperature-monitoring-system
microcontroller-based system to display temperature, trigger alarms, and indicate temperature ranges using LEDs and 4Digit Seven Segment display.

## 3D view of v1.5
![System Diagram](https://github.com/NipunaMuhandiram/Microcontroller-based-temperature-monitoring-system/blob/main/v1.5/snaps/v1_5%20(2).png?raw=true)

![System Diagram](https://github.com/NipunaMuhandiram/Microcontroller-based-temperature-monitoring-system/blob/main/v1.5/snaps/v1_5%20(5).png?raw=true)

## 3D view of v1.4

![System Diagram](https://github.com/NipunaMuhandiram/Microcontroller-based-temperature-monitoring-system/blob/main/v1.4/Snips/v1_4%20(3).png?raw=true)

![System Diagram](https://github.com/NipunaMuhandiram/Microcontroller-based-temperature-monitoring-system/blob/main/v1.4/Schematic/v1.4.png?raw=true)
## Overview

This project involves designing a microcontroller-based temperature monitoring system that displays temperature on a 7-segment display, triggers alarms at specified thresholds, and indicates temperature ranges using LEDs. The system will also support switching between Celsius and Fahrenheit displays and toggle between temperature display and range indication modes.

## Components

1. Microcontroller (ATmega328p ic)
2. 7-Segment Display
3. Push Buttons (5)
4. LEDs (5 for temperature indication, 1 for alarm)
5. Resistors
6. Breadboard and jumper wires
## Tasks
### Task 1: Temperature Display on 7-Segment Display

- **Increment and Decrement Temperature**: Use two push buttons to simulate the increment and decrement of the temperature.
- **Display Temperature**: The current temperature is displayed on a 7-segment display. The display alternates between Celsius and Fahrenheit.
### Task 2: Temperature Alarm

- **Threshold Temperature**: An alarm is triggered when the temperature exceeds 40°C.
- **Alarm Signa**l: A separate LED blinks to signal the alarm.
- **Alarm Reset**: A push button is used to reset or acknowledge the alarm.
- **Alarm Functionality**: The alarm works in both display modes.
### Task 3: Temperature Range Indication with LEDs

- **Temperature Ranges and LEDs**:
	- LED 1: Cold (below 15°C)
	- LED 2: Normal (15°C to 25°C)
	- LED 3: Warmth (26°C to 35°C)
	- LED 4: Hot (above 35°C)
- **LED Brightness: The brightness of the LEDs changes within the specified temperature ranges**:
	- Increase brightness of LED 1 in 0-15°C range.
	- Increase brightness of LED 2 in 16-25°C range.
	- Increase brightness of LED 3 in 26-35°C range.
	- Increase brightness of LED 4 in 36-40°C range.
	- Blink the alarm LED when the temperature is above 40°C.
### Task 4: Toggle Between Celsius and Fahrenheit

- **Push Button**: A push button is used to switch between Celsius and Fahrenheit display on the 7-segment display.
- **Conversion Logic**: The system includes logic to correctly convert and display temperatures in both units.
### Task 5: Mode Switching

- **Mode Toggle**: A push button toggles between different display modes:
- **Mode 1**: Display the current temperature on the 7-segment display.
- **Mode 2**: Show the temperature range indication using the LEDs.
- **Button Press**: Each press of the button switches to the next mode.
