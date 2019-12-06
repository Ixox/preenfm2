PFM2_VERSION_NUMBER=2.11
PFM2_BIN_NUMBER=$(subst .,,${PFM2_VERSION_NUMBER})
PFM2_BOOTLOADER_VERSION_NUMBER=1.11
PFM2_VERSION=\"${PFM2_VERSION_NUMBER}\"
PFM2_BOOTLOADER_VERSION=\"${PFM2_BOOTLOADER_VERSION_NUMBER}\"

ELF_BOOTLOADER=build/p2_boot_${PFM2_BOOTLOADER_VERSION_NUMBER}.elf
BIN_BOOTLOADER=build/p2_boot_${PFM2_BOOTLOADER_VERSION_NUMBER}.bin
SYMBOLS_BOOTLOADER=build/symbols_p2_boot_${PFM2_BOOTLOADER_VERSION_NUMBER}.txt

ELF_FIRMWARE=build/p2_${PFM2_VERSION_NUMBER}.elf
ELF_FIRMWARE_O=build/p2_${PFM2_VERSION_NUMBER}o.elf
BIN_FIRMWARE=build/p2_${PFM2_BIN_NUMBER}.bin
BIN_FIRMWARE_O=build/p2_${PFM2_BIN_NUMBER}o.bin
SYMBOLS_FIRMWARE=build/symbols_p2_${PFM2_VERSION_NUMBER}.txt
SYMBOLS_FIRMWARE_O=build/symbols_p2_${PFM2_VERSION_NUMBER}o.txt

BIN_SYSEX=build/p2_${PFM2_VERSION_NUMBER}.syx

C      = arm-none-eabi-gcc
CC      = arm-none-eabi-c++
LD      = arm-none-eabi-ld -v
CP      = arm-none-eabi-objcopy
OD      = arm-none-eabi-objdump
AS      = arm-none-eabi-as


SRC_FIRMWARE = src/PreenFM.cpp \
	src/PreenFM_irq.c \
	src/PreenFM_init.c \
	src/usb/usbKey_usr.c \
	src/usb/usbMidi_usr.c \
	src/usb/usbd_midi_desc.c \
	src/usb/usb_bsp.c \
	src/usb/usbd_midi_core.c \
	src/library/STM32_USB_OTG_Driver/src/usb_core.c \
	src/library/STM32_USB_OTG_Driver/src/usb_hcd.c \
	src/library/STM32_USB_OTG_Driver/src/usb_hcd_int.c \
	src/library/STM32_USB_HOST_Library/Core/src/usbh_core.c \
	src/library/STM32_USB_HOST_Library/Core/src/usbh_hcs.c \
	src/library/STM32_USB_HOST_Library/Core/src/usbh_ioreq.c \
	src/library/STM32_USB_HOST_Library/Core/src/usbh_stdreq.c \
	src/library/STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_core.c \
	src/library/STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_bot.c \
	src/library/STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_scsi.c \
	src/library/STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_fatfs.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/misc.c \
	src/library/STM32_USB_OTG_Driver/src/usb_dcd.c \
	src/library/STM32_USB_OTG_Driver/src/usb_dcd_int.c \
	src/library/STM32_USB_Device_Library/Core/src/usbd_core.c \
	src/library/STM32_USB_Device_Library/Core/src/usbd_ioreq.c \
	src/library/STM32_USB_Device_Library/Core/src/usbd_req.c \
	src/third/system_stm32f4xx.c \
	src/utils/Hexter.cpp \
	src/utils/RingBuffer.cpp \
	src/hardware/LiquidCrystal.cpp \
	src/hardware/Menu.cpp \
	src/hardware/FMDisplay.cpp \
	src/hardware/Encoders.cpp \
	src/hardware/LiquidCrystal.cpp \
	src/filesystem/ComboBank.cpp \
	src/filesystem/ConfigurationFile.cpp \
	src/filesystem/DX7SysexFile.cpp \
	src/filesystem/PatchBank.cpp \
	src/filesystem/ScalaFile.cpp \
	src/filesystem/Storage.cpp \
	src/filesystem/FileSystemUtils.cpp \
	src/filesystem/PreenFMFileType.cpp \
	src/filesystem/UserWaveform.cpp \
	src/midi/MidiDecoder.cpp \
	src/synth/Env.cpp \
	src/synth/Lfo.cpp \
	src/synth/LfoEnv.cpp \
	src/synth/LfoEnv2.cpp \
	src/synth/LfoOsc.cpp \
	src/synth/LfoStepSeq.cpp \
	src/synth/Matrix.cpp \
	src/synth/Osc.cpp \
	src/synth/Presets.cpp \
	src/synth/Synth.cpp \
	src/synth/SynthMenuListener.cpp \
	src/synth/SynthParamListener.cpp \
	src/synth/SynthStateAware.cpp \
	src/synth/SynthState.cpp \
	src/synth/Voice.cpp \
	src/synth/Timbre.cpp \
	src/synth/Common.cpp \
	src/library/fat_fs/src/ff.c \
	src/library/fat_fs/src/fattime.c  \
	src/midipal/note_stack.cpp \
	src/midipal/event_scheduler.cpp


SRC_BOOTLOADER = src/bootloader/BootLoader.cpp \
	src/bootloader/usb_storage_usr.c \
	src/bootloader/usbd_storage_desc.c \
	src/bootloader/usbd_storage.c \
	usr/usb/usbKey_usr.c \
	usr/usb/usb_bsp.c \
	src/hardware/Encoders.cpp \
	src/hardware/LiquidCrystal.cpp \
	src/filesystem/Storage.cpp \
	src/filesystem/FirmwareFile.cpp \
	src/synth/Common.cpp \
	src/third/system_stm32f4xx.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c   \
	src/library/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
	src/library/fat_fs/src/ff.c \
	src/library/fat_fs/src/fattime.c \
	src/library/STM32_USB_OTG_Driver/src/usb_core.c \
	src/library/STM32_USB_OTG_Driver/src/usb_hcd.c \
	src/library/STM32_USB_OTG_Driver/src/usb_hcd_int.c \
	src/library/STM32_USB_OTG_Driver/src/usb_dcd.c \
	src/library/STM32_USB_OTG_Driver/src/usb_dcd_int.c 	\
	src/library/STM32_USB_HOST_Library/Core/src/usbh_core.c \
	src/library/STM32_USB_HOST_Library/Core/src/usbh_hcs.c \
	src/library/STM32_USB_HOST_Library/Core/src/usbh_ioreq.c \
	src/library/STM32_USB_HOST_Library/Core/src/usbh_stdreq.c \
	src/library/STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_core.c \
	src/library/STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_bot.c \
	src/library/STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_scsi.c \
	src/library/STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_fatfs.c \
	src/library/STM32_USB_Device_Library/Core/src/usbd_core.c \
	src/library/STM32_USB_Device_Library/Core/src/usbd_ioreq.c \
	src/library/STM32_USB_Device_Library/Core/src/usbd_req.c \
	src/library/STM32_USB_Device_Library/Class/msc/src/usbd_msc_bot.c \
	src/library/STM32_USB_Device_Library/Class/msc/src/usbd_msc_core.c \
	src/library/STM32_USB_Device_Library/Class/msc/src/usbd_msc_data.c \
	src/library/STM32_USB_Device_Library/Class/msc/src/usbd_msc_scsi.c \
	src/library/STM32F4xx_StdPeriph_Driver/src/misc.c

INCLUDESDIR = -I./src/third/ -I./src/library/STM32_USB_HOST_Library/Class/MSC/inc -I./src/library/STM32_USB_HOST_Library/Core/inc -I./src/library/STM32_USB_OTG_Driver/inc -I./src/library/fat_fs/inc -I./src/library/STM32_USB_Device_Library/Core/inc -I./src/library/STM32_USB_Device_Library/Class/midi/inc -I./src/library/STM32F4xx_StdPeriph_Driver/inc/ -I./src/library/CMSIS/Include/ -I./src/utils/ -I./src/filesystem -I./src/hardware -I./src/usb -I./src/synth -I./src/midi -I./src/midipal -I./src -I./src/library/STM32_USB_Device_Library/Class/msc/inc
SMALLBINOPTS = -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions  -Wl,--gc-sections

#
DEFINE = -DPFM2_VERSION=${PFM2_VERSION} -DPFM2_BOOTLOADER_VERSION=${PFM2_BOOTLOADER_VERSION}

ifeq ($(DEBUG),1)
DEFINE += -DDEBUG
endif
CFLAGS = -Ofast $(INCLUDESDIR) -c -fno-common   -g  -mthumb -mcpu=cortex-m4 -mfloat-abi=hard $(SMALLBINOPTS) $(DEFINE) -fsigned-char
AFLAGS  = -ahls -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
LFLAGS  = -Tlinker/stm32f4xx.ld  -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -gc-sections    --specs=nano.specs
LFLAGS_BOOTLOADER  = -Tlinker_bootloader/stm32f4xx.ld  -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -gc-sections --specs=nano.specs
CPFLAGS = -Obinary

STARTUP = build/startup_firmware.o
STARTUP_BOOTLOADER = build/startup_bootloader.o
OBJDIR = ./build/


OBJTMP_FIRMWARE = $(addprefix $(OBJDIR),$(notdir $(SRC_FIRMWARE:.c=.o)))
OBJ_FIRMWARE = $(OBJTMP_FIRMWARE:.cpp=.o)

OBJTMP_BOOTLOADER = $(addprefix $(OBJDIR),$(notdir $(SRC_BOOTLOADER:.c=.o)))
OBJ_BOOTLOADER = $(OBJTMP_BOOTLOADER:.cpp=.o)


all:
	@echo "You must chose a target"
	@echo "   clean : remove build directory"
	@echo "   pfm : build pfm2 firmware"
	@echo "   pfmo : build pfm2 overclocked firmware"
	@echo "   boot : build pfm2 bootloader"
	@echo "   install : flash firmware through the programming interface"
	@echo "   installo : flash overclocked firmware through the programming interface"
	@echo "   installdfu : flash firmware through DFU"
	@echo "   installdfuo : flash overclocked firmware through DFU"
	@echo "   installboot : flash bootloader"
	@echo "   installbootdfu : flash bootloader through DFU"
	@echo "   zip : create zip with all inside"

zip: pfm2_$(PFM2_VERSION_NUMBER).zip

release: zip
	mkdir -p release/fw_$(PFM2_VERSION_NUMBER)
	cp build/install_firmware.cmd release/fw_$(PFM2_VERSION_NUMBER)/
	cp build/install_firmware_overclocked.cmd release/fw_$(PFM2_VERSION_NUMBER)/
	cp $(BIN_FIRMWARE) release/fw_$(PFM2_VERSION_NUMBER)/
	cp $(BIN_FIRMWARE_O) release/fw_$(PFM2_VERSION_NUMBER)/
	
	
pfm2_$(PFM2_VERSION_NUMBER).zip :
	echo "dfu-util -a0 -d 0x0483:0xdf11 -D $(notdir $(BIN_FIRMWARE)) -s 0x8040000" > build/install_firmware.cmd
	echo "dfu-util -a0 -d 0x0483:0xdf11 -D $(notdir $(BIN_FIRMWARE_O)) -s 0x8040000" > build/install_firmware_overclocked.cmd
#	echo "dfu-util -a0 -d 0x0483:0xdf11 -D $(notdir $(BIN_BOOTLOADER)) -s 0x8000000" > build/install_bootloader.cmd
	zip pfm2_$(PFM2_VERSION_NUMBER).zip build/*.bin build/*.syx build/*.cmd




pfm: $(BIN_FIRMWARE)

pfmo: CFLAGS += -DOVERCLOCK
pfmo: $(BIN_FIRMWARE_O)

boot: CFLAGS += -DBOOTLOADER -I./src/bootloader -Os
boot: $(BIN_BOOTLOADER)

install: $(BIN_FIRMWARE)
	st-flash write $(BIN_FIRMWARE) 0x08040000

installo: $(BIN_FIRMWARE_O)
	st-flash write $(BIN_FIRMWARE_O) 0x08040000

installdfu: $(BIN_FIRMWARE)
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_FIRMWARE) -s 0x8040000

installdfuo: $(BIN_FIRMWARE_O)
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_FIRMWARE_O) -s 0x8040000

installboot:boot
	st-flash write $(BIN_BOOTLOADER) 0x08000000

installbootdfu:boot
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_BOOTLOADER) -s 0x8000000

installall:
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_BOOTLOADER) -s 0x8000000
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_FIRMWARE) -s 0x8040000

installallo:
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_BOOTLOADER) -s 0x8000000
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_FIRMWARE_O) -s 0x8040000


sysex : sysex/sysex.class
	java -classpath sysex sysex $(BIN_FIRMWARE)
	java -classpath sysex sysex $(BIN_FIRMWARE_O)

sysex/sysex.class : sysex/sysex.java
	javac sysex/sysex.java

amidi:
	amidi -p hw:3,0,0 -s $(BIN_SYSEX)

$(BIN_FIRMWARE): $(ELF_FIRMWARE)
	$(CP) -O binary $^ $(BIN_FIRMWARE)
	ls -l $(BIN_FIRMWARE)

$(BIN_FIRMWARE_O): $(ELF_FIRMWARE_O)
	$(CP) -O binary $^ $(BIN_FIRMWARE_O)
	ls -l $(BIN_FIRMWARE_O)

$(ELF_FIRMWARE): $(OBJ_FIRMWARE) $(STARTUP)
	$(CC) $(LFLAGS) $^ -o $@
	arm-none-eabi-nm -l -S -n $(ELF_FIRMWARE) > $(SYMBOLS_FIRMWARE)
	arm-none-eabi-readelf -S $(ELF_FIRMWARE)

$(ELF_FIRMWARE_O): $(OBJ_FIRMWARE) $(STARTUP)
	@$(CC) $(LFLAGS) $^ -o $@
	arm-none-eabi-nm -l -S -n $(ELF_FIRMWARE_O) > $(SYMBOLS_FIRMWARE_O)
	arm-none-eabi-readelf -S $(ELF_FIRMWARE_O)


$(BIN_BOOTLOADER): $(ELF_BOOTLOADER)
	$(CP) -O binary $^ $(BIN_BOOTLOADER)
	ls -l $(BIN_BOOTLOADER)

$(ELF_BOOTLOADER): $(OBJ_BOOTLOADER) $(STARTUP_BOOTLOADER)
	$(CC) $(LFLAGS_BOOTLOADER) $^ -o $@
	arm-none-eabi-nm -l -S -n $(ELF_BOOTLOADER) > $(SYMBOLS_BOOTLOADER)
	arm-none-eabi-readelf -S $(ELF_BOOTLOADER)


$(STARTUP): linker/startup_stm32f4xx.s
	$(AS) $(AFLAGS) -o $(STARTUP) linker/startup_stm32f4xx.s > linker/startup_stm32f4xx.lst

$(STARTUP_BOOTLOADER): linker_bootloader/startup_stm32f4xx.s
	$(AS) $(AFLAGS) -o $(STARTUP_BOOTLOADER) linker_bootloader/startup_stm32f4xx.s > linker_bootloader/startup_stm32f4xx.lst


build/%.o: src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/bootloader/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/bootloader/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/usb/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/usb/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/third/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/synth/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/hardware/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/filesystem/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/midi/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/utils/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

build/%.o: src/library/STM32_USB_OTG_Driver/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/library/STM32_USB_HOST_Library/Core/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/library/STM32_USB_HOST_Library/Class/MSC/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/library/STM32_USB_Device_Library/Core/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/library/STM32_USB_Device_Library/Class/msc/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/library/STM32F4xx_StdPeriph_Driver/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/library/fat_fs/src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/midipal/%.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS)  $< -o $@


clean:
	@ echo ".clean"
	rm -f *.lst build/*.o pfm2_$(PFM2_VERSION_NUMBER).zip

cleanall:
	@ echo ".clean all"
	rm -rf *.lst build/*
