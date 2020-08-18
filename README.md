# preenfm2 firmware

Ixox/preenfm2 is the official repository of the preenfm2 firmware.

You can find here the compiled firmware, its code source and some hardware files for the PCB, MCU board and cases.

To flash the preenfm2 for the first time, [follow these instructions](https://github.com/Ixox/preenfm2/tree/master/flash)


## Compiling the firmware

To compile the firmware, you'll need [arm-gcc version 4.7](https://launchpad.net/gcc-arm-embedded/+milestone/4.7-2014-q2-update)

Add the bin directory to your PATH, and run **'make'**, you'll get the list of the available targets.

```bash
$ make
You must chose a target 
Don't forget to clean between different build targets
   clean : clean build directory
   pfm : build pfm2 firmware
   pfmo : build pfm2 overclocked firmware
   pfmcv : build pfm2 firmware for Eurorack 
   pfmcvo : build pfm2 overclocked firmware for Eurorack
   installdfu : flash last compiled firmware through DFU
   zip : create zip with all inside
```
Since some refactoring, the bootloader does not compile anymore. But it's available in its binary format.

Then put your preenfm2 in [bootloader mode](http://ixox.fr/preenfm2/manual/upgrade-firmware/). Look at DFU part 4.

To flash the firmware on the preenfm2 using the DFU protocol :

```bash
make installdfu
```

Once it's done, unplug the power cable and plug it back.

## New Filters in 2.11

Many effects have been added in the firmware 2.11. They were coded by [Toltekradiation](http://ixox.fr/forum/index.php?topic=69544.0).

His github repo is [here](https://github.com/pvig/preenfm2). You'll find there some description of the different effects.
