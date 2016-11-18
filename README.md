## preenFM2

Ixox/preenfm2 is the official preenfm2 repository.

You can find here the sources of the firmware as well as the hardware files for the PCB, MCU board and cases.

If you think something is missing or bot clear, please contact me.

To compile, you'll need [arm-gcc version 4.7] (https://launchpad.net/gcc-arm-embedded/+milestone/4.7-2012-q4-major)
Add the bin directory to your PATH, and run make, you'll get the list of available target.

### If you don't have the preenfm2 bootlader installed

[Read this] (http://ixox.fr/preenfm2/build-it/burn-firmware/)

### If you have a working bootloader 

> make pfm

to build the firmware.

Then put your preenfm2 in [bootloader mode](http://ixox.fr/preenfm2/manual/upgrade-firmware/). Look at DFU part 4.

And run :

> make installdfu

After 20 seconds or so, your new firmware is on your preenfm2. Unpower it and power it back.


