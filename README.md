# M&M Sorter using OpenCV and Arduino

An M&M sorter built from spare parts of an old printer (Epson C43UX), an Arduino, a webcam and OpenCV. The webcam and OpenCV pick out each M&M's colour, and the Arduino drives the printer's print-head stepper and two servos to drop it in the right bin.

- `mnmSorter.cpp`: OpenCV (C++) code that runs on the PC
- `mnmSorter.ino`: Arduino code

Videos:
- Part 1, how it's built: https://youtu.be/wAQINPSFfOg
- Part 2, see it running: https://youtu.be/eY7hiALKnTo

Project page: https://alexinflatland.com/projects/m-m-sorting-using-opencv-and-arduino-part-1/

## Notes from the original build

- The print head is moved by the printer's own bipolar stepper (EM-462), driven by an SN754410NE H-bridge. It steps 7.5° per step, so 48 steps per revolution. Set it to 48 in the code, because 50 makes it skip steps.
- The motor's coil resistance is 9.2 Ω and it's rated at about 1 A, which puts it at roughly 9 V.
- The printer's power supply gives about 36 V. A 7809 regulator brings it down to 9 V but runs very hot at that input, and an LM317 would be a better choice.
