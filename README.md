# Arduino-Interactive-LED-Strip

This project features the hardware and software development of a multi-function LED strip controller!

The light strip has 3 different modes:
1. HSV Static Colour Set
2. Patterned Light Show
3. Music Wave Visualizer

User can switch between the 3 modes by pushing a button. In each mode, parameters can be adjusted by turning the 3 potentiometer knobs.

Here are the functions of each knob in each mode:

| Mode  | Knob 1 | Knob 2 | Knob 3 |
| -------- | ---- | ---- | ---- |
| HSV Static Colour Set | Hue | Saturation | Brightness |
| Patterned Light Show | Colour Pattern | Scramble Effect | Brightness |
| Music Wave Visualizer | Wave Point Source | Volume Sensitivity | Brightness |

## Software

The arduino nano code for this controller. Simple code using FastLED library and interrupts for switching modes.

## Hardware

PCB designed in KiCAD to recreate this controller! Complete bill of materials provided. 
Can be assembled for about 30 - 70 CAD depending on how much you spend for PCB fabrication and the LED strip.
