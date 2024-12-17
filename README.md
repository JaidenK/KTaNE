# KTaNE
Keep Talking and Nobody Explodes

# Build Instructions
1. Arduino IDE version 2.2.1
2. Clear out your Arduino libraries folder.
    - ex: "C:\Users\kingj\OneDrive\Documents\Arduino\libraries\" should exist and be empty
3. Copy the contents of the repo's libraries folder into your Arduino install's libraries folder, excluding libraries that do not apply to the desired module.
    - ex: For compiling the Sketchpad module, you would copy ES_Framework, KTaNE, and KTaNE_Sketchpad but NOT KTaNE_Timer or other Module folders
4. Select "Arduino Uno" in the Aruino IDE's board selection menu

The code should now build.

# Programming Instructions
0. Refer to https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoToBreadboard/
    - See also: https://www.instructables.com/Configure-Arduino-IDE-for-Atmega-328P-to-Use-8MHz-/
    - See also: https://www.youtube.com/watch?v=dcLeKj00t_I
1. Get an Aruino Uno R3 and upload the "ArduinoISP" example sketch to it.
3. Connect the 6-pin ISP header on the Arduino to the 6-pin ISP header on the Module, with the exception of the RESET pin.
4. Connect pin 10 on the Arduino to the Reset pin of the Module's ISP header.
    - Pin 10 is the Arduino's default Slave Select pin for the ArduinoISP sketch.
5. Set the Arduino IDE's programmer to "Arduino as ISP" and select "ATMega328P on a breadboard (8 MHz internal clock)" for the board
6. Use "Sketch > Upload Using Programmer" menu command to upload the code
