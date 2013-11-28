PFM2_VERSION_NUMBER=0.9h
PFM2_BIN_NUMBER=$(subst .,,${PFM2_VERSION_NUMBER})
PFM2_BOOTLOADER_VERSION_NUMBER=1.01
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


SRC_FIRMWARE = PreenFM.cpp \
	PreenFM_irq.c \
	PreenFM_init.c \
	usbKey_usr.c \
	usbMidi_usr.c \
	usbd_preenFM_desc.c \
	system_stm32f4xx.c \
	usb_bsp.c \
	fat_fs/src/ff.c \
	fat_fs/src/fattime.c \
	STM32_USB_OTG_Driver/src/usb_core.c \
	STM32_USB_OTG_Driver/src/usb_hcd.c \
	STM32_USB_OTG_Driver/src/usb_hcd_int.c \
	STM32_USB_HOST_Library/Core/src/usbh_core.c \
	STM32_USB_HOST_Library/Core/src/usbh_hcs.c \
	STM32_USB_HOST_Library/Core/src/usbh_ioreq.c \
	STM32_USB_HOST_Library/Core/src/usbh_stdreq.c \
	STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_core.c \
	STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_bot.c \
	STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_scsi.c \
	STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_fatfs.c \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c \
	STM32F4xx_StdPeriph_Driver/src/misc.c \
	STM32_USB_OTG_Driver/src/usb_dcd.c \
	STM32_USB_OTG_Driver/src/usb_dcd_int.c \
	STM32_USB_Device_Library/Class/midi/usbd_midi_core.c \
	STM32_USB_Device_Library/Core/usbd_core.c \
	STM32_USB_Device_Library/Core/usbd_ioreq.c \
	STM32_USB_Device_Library/Core/usbd_req.c \
	src_synth/Env.cpp \
	src_synth/FMDisplay.cpp \
	src_synth/Hexter.cpp \
	src_synth/Lfo.cpp \
	src_synth/LfoEnv.cpp \
	src_synth/LfoEnv2.cpp \
	src_synth/LfoOsc.cpp \
	src_synth/LfoStepSeq.cpp \
	src_synth/LiquidCrystal.cpp \
	src_synth/Matrix.cpp \
	src_synth/Menu.cpp \
	src_synth/MidiDecoder.cpp \
	src_synth/Osc.cpp \
	src_synth/Presets.cpp \
	src_synth/RingBuffer.cpp \
	src_synth/Synth.cpp \
	src_synth/SynthMenuListener.cpp \
	src_synth/SynthParamListener.cpp \
	src_synth/SynthStateAware.cpp \
	src_synth/SynthState.cpp \
	src_synth/Voice.cpp \
	src_synth/PresetUtil.cpp \
	src_synth/Encoders.cpp \
	src_synth/Timbre.cpp \
	src_synth/LiquidCrystal.cpp \
	src_synth/UsbKey.cpp \
	src_synth/Storage.cpp \
	src_synth/Common.cpp
	
#	src_synth/FM.cpp 

SRC_BOOTLOADER = BootLoader.cpp \
	Encoders.cpp \
	LiquidCrystal.cpp \
	src_synth/UsbKey.cpp \
	src_synth/Storage.cpp \
	src_synth/LiquidCrystal.cpp \
	src_synth/Common.cpp \
	system_stm32f4xx.c \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c   \
	STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
	usbKey_usr.c \
	usb_bsp.c \
	fat_fs/src/ff.c \
	fat_fs/src/fattime.c \
	STM32_USB_OTG_Driver/src/usb_core.c \
	STM32_USB_OTG_Driver/src/usb_hcd.c \
	STM32_USB_OTG_Driver/src/usb_hcd_int.c \
	STM32_USB_HOST_Library/Core/src/usbh_core.c \
	STM32_USB_HOST_Library/Core/src/usbh_hcs.c \
	STM32_USB_HOST_Library/Core/src/usbh_ioreq.c \
	STM32_USB_HOST_Library/Core/src/usbh_stdreq.c \
	STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_core.c \
	STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_bot.c \
	STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_scsi.c \
	STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_fatfs.c \
	STM32F4xx_StdPeriph_Driver/src/misc.c 

 
INCLUDESDIR = -I./include  -I./include_synth  -I./STM32F4xx_StdPeriph_Driver/inc/ -I./LiquidCrystal -I./STM32_USB_HOST_Library/Class/MSC/inc -I./STM32_USB_HOST_Library/Core/inc -I./STM32_USB_OTG_Driver/inc -I./fat_fs/inc -I./STM32_USB_Device_Library/Core/inc -I./STM32_USB_Device_Library/Class/midi/inc 
SMALLBINOPTS = -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions  -Wl,--gc-sections  

# 
DEFINE = -DPFM2_VERSION=${PFM2_VERSION} -DPFM2_BOOTLOADER_VERSION=${PFM2_BOOTLOADER_VERSION}
CFLAGS  =  -Os $(INCLUDESDIR) -c -fno-common   -g  -mthumb -mcpu=cortex-m4 -mfloat-abi=hard $(SMALLBINOPTS) $(DEFINE) -fsigned-char
CFLAGS_NOOPTIM  =   $(INCLUDESDIR) -c -fno-common   -g  -mthumb -mcpu=cortex-m4 -mfloat-abi=hard $(SMALLBINOPTS) $(DEFINE) -fsigned-char
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

pfm2_$(PFM2_VERSION_NUMBER).zip : 
	echo "dfu-util -a0 -d 0x0483:0xdf11 -D $(notdir $(BIN_FIRMWARE)) -R -s 0x8040000" > build/install_firmware.cmd
	echo "dfu-util -a0 -d 0x0483:0xdf11 -D $(notdir $(BIN_FIRMWARE_O)) -R -s 0x8040000" > build/install_firmware_overclocked.cmd
	echo "dfu-util -a0 -d 0x0483:0xdf11 -D $(notdir $(BIN_BOOTLOADER)) -R -s 0x8000000" > build/install_bootloader.cmd
	zip pfm2_$(PFM2_VERSION_NUMBER).zip build/*.bin build/*.syx build/*.cmd

pfm: clean $(BIN_FIRMWARE) 

pfmo: CFLAGS += -DOVERCLOCK
pfmo: clean $(BIN_FIRMWARE_O) 

boot: CFLAGS += -DBOOTLOADER -Os 
boot: clean $(BIN_BOOTLOADER)

install: $(BIN_FIRMWARE)
	st-flash write $(BIN_FIRMWARE) 0x08040000

installo: $(BIN_FIRMWARE_O)
	st-flash write $(BIN_FIRMWARE_O) 0x08040000

installdfu: $(BIN_FIRMWARE)
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_FIRMWARE) -R -s 0x8040000

installdfuo: $(BIN_FIRMWARE_O)
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_FIRMWARE_O) -R -s 0x8040000

installboot:boot
	st-flash write $(BIN_BOOTLOADER) 0x08000000

installbootdfu:boot
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_BOOTLOADER) -R -s 0x8000000

installall:
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_BOOTLOADER) -R -s 0x8000000
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_FIRMWARE) -R -s 0x8040000

installallo:
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_BOOTLOADER) -R -s 0x8000000
	dfu-util -a0 -d 0x0483:0xdf11 -D $(BIN_FIRMWARE_O) -R -s 0x8040000


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
	$(CC) $(CFLAGS) -DOVERCLOCK src/PreenFM.cpp -o build/PreenFM.o
	$(CC) $(CFLAGS) -DOVERCLOCK src/system_stm32f4xx.c -o build/system_stm32f4xx.o
	$(CC) $(LFLAGS) $^ -o $@
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


build/usbh_msc_%.o: STM32_USB_HOST_Library/Class/MSC/src/usbh_msc_%.c
	$(CC) $(CFLAGS) -fpermissive $< -o $@

build/stm32f4xx_%.o: ./STM32F4xx_StdPeriph_Driver/src/stm32f4xx_%.c
	$(CC) $(CFLAGS) $< -o $@

build/misc.o: ./STM32F4xx_StdPeriph_Driver/src/misc.c
	$(CC) $(CFLAGS) $< -o $@

build/usbh_%.o: STM32_USB_HOST_Library/Core/src/usbh_%.c
	$(CC) $(CFLAGS) -fpermissive $< -o $@

build/usbd_%.o: STM32_USB_Device_Library/Core/src/usbd_%.c
	$(CC) $(CFLAGS) -fpermissive $< -o $@

build/usb_%.o: STM32_USB_OTG_Driver/src/usb_%.c
	$(CC) $(CFLAGS) -fpermissive $< -o $@

build/usbd_midi_%.o: STM32_USB_Device_Library/Class/midi/src/usbd_midi_%.c
	$(CC) $(CFLAGS) -fpermissive $< -o $@

build/ff.o: ./fat_fs/src/ff.c
	$(CC) $(CFLAGS) -fpermissive $< -o $@

build/fattime.o: ./fat_fs/src/fattime.c
	$(CC) $(CFLAGS) -fpermissive $< -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $@

build/%.o: src_synth/%.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	@ echo ".clean"
	rm -f *.lst build/*.o pfm2_$(PFM2_VERSION_NUMBER).zip

cleanall:
	@ echo ".clean all"
	rm -rf *.lst build/*

