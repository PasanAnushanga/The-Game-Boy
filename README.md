# Game Boy Clone with Arduino

A custom Game Boy clone built using an Arduino, ST7920 128x64 display, rotary encoder, buttons, and a buzzer. This project replicates the functionality of a classic handheld game console, complete with directional buttons and sound effects!

## Features
- **128x64 Monochrome Display (ST7920)**
- **Rotary Encoder for Menu Selection**
- **Directional Buttons (Up, Right, Down, Left)**
- **Buzzer for Sound Effects**

## Components Used
- **Arduino Uno/Nano**
- **ST7920 128x64 Monochrome Display**
- **Rotary Encoder with Push Button**
- **4 Push Buttons for Directional Control**
- **Buzzer**

## Circuit Diagram

### Arduino Connections

#### Display (ST7920 128x64):
- **VCC**: Connect to Arduino 5V
- **GND**: Connect to Arduino GND
- **SCK (Clock)**: Connect to Arduino pin **13** (SPI SCK)
- **MOSI (Data)**: Connect to Arduino pin **11** (SPI MOSI)
- **CS (Chip Select)**: Connect to Arduino pin **10**
- **RST (Reset)**: Connect to Arduino pin **9**

#### Rotary Encoder:
- **CLK**: Connect to Arduino pin **6**
- **DT**: Connect to Arduino pin **7**
- **SW (Button)**: Connect to Arduino pin **8**
- **VCC**: Connect to Arduino 5V
- **GND**: Connect to Arduino GND

#### Directional Buttons:
- **Up Button**: Connect to Arduino pin **A0**
- **Right Button**: Connect to Arduino pin **A1**
- **Down Button**: Connect to Arduino pin **A2**
- **Left Button**: Connect to Arduino pin **A3**

#### Buzzer:
- **Positive (+)**: Connect to Arduino pin **5**
- **Negative (-)**: Connect to Arduino GND

### Wiring Diagram

(Include an image of the circuit diagram here if possible)

## Code Explanation

The code uses the `U8g2` library to handle the ST7920 display and `RotaryEncoder` library for the rotary encoder. The buttons are mapped to analog pins and used for game control input. The buzzer is used to generate sound effects for user interactions or game events.

## How to Upload

1. Connect the Arduino to your computer via USB.
2. Ensure you have the necessary libraries installed (U8g2, RotaryEncoder).
3. Upload the provided sketch to the Arduino.

## Libraries Required
- **U8g2**: For the ST7920 display.
- **RotaryEncoder**: For handling the rotary encoder.
- **Bounce2**: For debouncing the buttons.

You can install these libraries through the Arduino IDE Library Manager.

## License

This project is open-source under the MIT License. Feel free to modify and distribute it.

---

Happy coding! Enjoy your custom Game Boy project!
