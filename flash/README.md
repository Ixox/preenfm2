You may need to flash the memory of the preenfm if it was not done before.

The following need to be done only once. Once the bootloader is flashed, you may boot into it (hold a key while turning on the preenfm2) and flash the firmwares from it.

## Flash the STM32F405 on Windows


The easiest solution on windows is to download [DfuSe](https://www.st.com/en/development-tools/stsw-stm32080.html) from STMicroelectronics. It contains the flash programm and the usb required drivers.

![DfuSe](dfuse.png)

To have your preenfm2 visible in the top left "Available DFU devices" list, you have to enter bootloader mode.
1. Unplug the preenfm
2. Connect the 2 pins of the Boot0 pins of your preenfm2
3. Plug the preenfm

Then in the "Upgrade or Verify Action" click on Chose, select the [pfm2_flash.dfu](pfm2_flash.dfu) that should be in the current folder.
And click on Upgrade.

When it's done don't forget to unconnect the 2 pins of Boot0 before turning off and back on your preenfm.


## Flash the STM32F405 on MacOs or Linux

On those platforms you'll have to use "dfu-util".

On linux create a new file  /etc/udev/rules.d/46-stm32F405.rules and add this content :
```
   ATTRS{idProduct}=="df11", ATTRS{idVendor}=="0483", MODE="664", GROUP="plugdev"
```

First follow steps 1,2,3 above to enter bootloader mode.

Then flash the bootloader at the adress 0×8000000. (run install_bootloader.cmd from the firmware zip file or from the current folder)

```
dfu-util -a0 -d 0×0483:0xdf11 -D p2_boot_1.11.bin -R -s 0×8000000
```

Flash the firmware at the adress 0×8040000.  (run install_firmware.cmd from the firmware zip file)

```
dfu-util -a0 -d 0×0483:0xdf11 -D pfm2_211.bin -R -s 0×8040000
```
