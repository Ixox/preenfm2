## preenfm2

Ixox/preenfm2 is the official preenfm2 repository.

You can find here the sources of the firmware as well as the hardware files for the PCB, MCU board and cases.

If you think something is missing or not clear, please contact me.

If you don't have the preenfm2 bootlader installed, you need to flash it first, [read this] (http://ixox.fr/preenfm2/build-it/burn-firmware/)

To compile the firmware, you'll need [arm-gcc version 4.7] (https://launchpad.net/gcc-arm-embedded/+milestone/4.7-2012-q4-major)

Add the bin directory to your PATH, and run **'make'**, you'll get the list of the available targets.

**'make pfm'** builds the firmware.

Then put your preenfm2 in [bootloader mode](http://ixox.fr/preenfm2/manual/upgrade-firmware/). Look at DFU part 4.

**'make installdfu'** flash the firmware on the preenfm2 using the DFU protocol.

Once it's done, unplug the power cable and plug it back.
