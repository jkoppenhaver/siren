# Interrupt Driven Siren on the TI LaunchPad
This project demonstrates how to set up and run different types of interrupts on an ARM cortex M4F series microcontroller.  The siren uses PWM, timer interrupts, and button interrupts to keep the main loop empty.

## Hardware
This project was created on a Stellaris LM4F120 LaunchPad Evaluation Board.  The only other hardware needed for this project is a speaker.  A small piezo can be driven with the output on the board directly.  A small audio amplifier can also be used to drive any 4 or 8 ohm speaker.  In testing, when driven with 12V, a small mono audio amplifier kit and single 4 inch speaker was able to produce a sound that was too loud to run for very long.  It is unkown how long the speaker would last in that set-up and I would recommend experimenting with speakers you don't mind destroying.

## Software
This project was compiled and loaded onto the evaluation board using Texas Insturments free IDE, Code Composer Studio v8.  CCS can be downloaded from [TIs website](http://www.ti.com/tool/CCSTUDIO).
This project now uses driverLib from TI.  This is a driver library provided free by TI that provides API functions for most low level peripherals.   The code is much cleaner and easier to read when using driverlib so all code has been converted to use these functions when possible.
To install driverlib:
1. Download and install [StellarisWare](http://www.ti.com/tool/sw-ek-lm4f120xl) for the launchpad from TIs website.\
2. Include the 'inc' directory in your project.
+ In CCS, open the project properties and got to Build->ARM Compiler->Include Options.
+ Click the 'Add dir to #include search path button and find the 'inc' directory in the StellarisWare directory you just installed.
3. A good walkthrough for including StellarisWare in your CCS project can be found [here](https://hackaday.com/2012/10/14/using-stellarisware-with-the-launchpad/).
4. The processor must also be defined so StellarisWare knows which chip is being used.
+ Open the project properties and go to Build->ARM Compiler->Advanced Options->Predefined Symbols
+ Next to 'Pre-define NAME(--define, -D)', click the add button and add 'PART_LM4F120H5QR' without the quotes.
CCS should now be set up to use driverlib.