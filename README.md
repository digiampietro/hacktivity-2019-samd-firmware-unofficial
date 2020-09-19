# What is this?
This is the firmware running on the SAMD co-processor found on the Disobey 2019 badge

# How do I compile this firmware?
First install version 1.6.18 of the "Arduino SAMD boards" board support package using the Arduino boards manager.
Then navigate to ~/.arduino15/packages/arduino/hardware/samd and replace the 1.6.18 folder with the folder found in the "arduino-board-support.zip" archive.

Then open "firmware.ino" with the Arduino IDE and click Sketch > Export compiled binary

# How do I flash the binary to my board?
First solder on SW1 and SW2 if you did not do that already.
Then hold SW2 (the SAMD bootloader trigger pin) while pressing shortly on SW1 (the SAMD reset pin).
This will place the SAMD into download mode.

Then execute the following command:

```
sudo dfu-util -D firmware.ino.disobey.bin -R
```

# How do I use it?

The firmware allows for connecting to the serial port of the ESP32 at 115200 baud.

The ESP32 can access the touch buttons, LEDs, speaker and battery voltage readings over I2C.

# The I2C protocol



# More information
[The github repository for the Disobey 2019 badge](https://github.com/disobeyfi/badge-2019)

# License and authors

The firmware in this repository was originally written for Disobey.fi by Kliment and patched up a bit by Renze.

It was patched by Valerio Di Giampietro for the Hacktivity 2019 badge (unofficial firmware), and to use this badge with the Arduino IDE, instead of microPython.

Copyright 2019 badge.team
Copyright 2020 Valerio Di Giampietro

All code found in the firmware folder in this repository is licensed under the Apache license,
as found below.

Our dependencies remain licensed under their respective original licenses.

This project contains the following differently licensed files:

The Arduino board support files (the arduino-board-support.zip archive)
The SAMD bootloader by Alex Taradov and Peter Lawrence
