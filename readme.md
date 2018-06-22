# Interrupt Driven Siren on the TI LaunchPad
This project demonstrates how to set up and run different types of interrupts on an ARM cortex M4F series microcontroller.  The siren uses PWM, timer interrupts, and button interrupts to keep the main loop empty.

## Hardware
This project was created on a Stellaris LM4F120 LaunchPad Evaluation Board.  The only other hardware needed for this project is a speaker.  A small piezo can be driven with the output on the board directly.  A small audio amplifier can also be used to drive any 4 or 8 ohm speaker.  In testing, when driven with 12V, a small mono audio amplifier kit and single 4 inch speaker was able to produce a sound that was too loud to run for very long.  It is unkown how long the speaker would last in that set-up and I would recommend experimenting with speakers you don't mind destroying.

## Software
This project was compiled and loaded onto the evaluation board using Texas Insturments free IDE, Code Composer Studio v8.  CCS can be downloaded from [TIs website](http://www.ti.com/tool/CCSTUDIO).