# Autonomous-Irrigation

To build this circuit you need the following items:

- 1x Arduino Uno Rev3
- 1x LCD4884 shield (with a 5 degree of freedom joystick)
- 1x Data logging shield
- 3x IRF3205 N-Channel Power MOSFETs
- 3x 1N4001 flyback diode
- 3x Plastic water solenoid valve (12V)
- 1x Power supply (12V DC)
- Breadboard and wires

To simplify the schematics I will not represent the LCD4884 shield and the data logging shield. They are stacked in the following order:<br/>
**Bottom**: Arduino Uno Rev3<br/>
**Middle**: Data logging shield<br/>
**Top**: LCD4884 shield<br/>
<br/>
![alt text](resources/Autonomous-Irrigation_bb.png?raw=true)

Some important remarks that should not be forgotten:

1) Before compiling the code you need to download, uncompress and install the following libraries: [RTClib](https://github.com/adafruit/RTClib) and [LCD4884](https://github.com/alt236/LCD4884---Arduino). To install simply copy them into the “libraries” folder. Alternatively you can read the official tutorial: [Installing Additional Arduino Libraries](https://github.com/alt236/LCD4884---Arduino).

2) The pinout for the IRF3205 N-Channel Power MOSFETs is the following:<br/>
![alt text](resources/to220.png?raw=true)

3) The 12V power supply is used to power the Arduino Uno (Vin pin) and the electrovalves. Keep in mind that red wires operate at 5V and orange wires operate at 12V.
Even though there is no 5V electronics represented in the circuit (besides the LCD4884 and data logging shields) I decided to represent the 5V line (red wires) for future reference.

4) Don’t forget to add the 1N4001 flyback diodes (one for each electrovalve). They are very important because the switching of an inductive load will originate transients (voltage spikes) which might damage sensitive electronic components.

5) Make sure that on your data logging shield SDA is connected to A4 and SCL is connected to A5. If these pins are not connected the RTC (DS1307) will not be able to communicate with the Arduino Uno.

6) On the Arduino Uno, digital pin 13 is the same as ICSP SCK, pin 12 is ICSP MISO and pin 11 is ICSP MOSI. However, this is true only for the Arduino Uno.
Other Arduinos have different connections and pinouts. This might also be true for Arduinos clones. These pins are used for the SD card communication, which I am not using for this project.
So, if you plan to add an SD card in this project you must be aware of which Arduino and data logging shield you are using and perform the connections accordingly.

7) The communication between the LCD4884 shield and the Arduino Uno uses pins D2, D3, D4, D5 and D6. There is also a 5 degree of freedom joystick (or 6 states joystick) which uses pin A0. The LCD backlight is controlled by pin D7 (not used in this project).

8) The 5 degree of freedom joystick (or 6 states joystick: up, down, left, right, pressed and unpressed) relies on a set of 5 resistors with each one having a different value. This method is very useful because it is only needed one analog input to operate the joystick.
However, if the resistors on your LCD4884 have different values you must adapt this code to make it work. Another option is to buy the exact same LCD4884 shield as I am using for this project (see note at the bottom).
If you want to use another LCD4884 shield make sure you get the following readings from each position:<br/>
**Unpressed:** 1003~1023<br/>
**Pressed:** 135~155<br/>
**Up:** 735~755<br/>
**Down:** 320~340<br/>
**Left:** 0~20<br/>
**Right:** 495~515<br/>

9) Make sure that you use a good power supply with enough power to operate the electrovalves.

**Note**: If you do not have the skills to debug and adapt your hardware to my code, I recommend you to use the following boards:

Arduino Uno Rev3. Buy it [here](https://store.arduino.cc/arduino-uno-smd-rev3)<br/>
![alt text](resources/ArduinoUnoRev3SMD.jpg?raw=true)

LCD4884 shield (with a 5 degree of freedom joystick). Buy it [here](https://www.dfrobot.com/product-347.html)<br/>
![alt text](resources/LCD4884.jpg?raw=true)

Data logging shield. Buy it [here](http://hobbycomponents.com/shields/587-data-logger-shield)<br/>
![alt text](resources/DataLoggingShield.jpg?raw=true)

Plastic Water Solenoid Valve (12V). Buy it [here](https://www.adafruit.com/product/997)<br/>
![alt text](resources/SolenoidValve.jpg?raw=true)

