# This is a prototype Makefile. Modify it according to your needs.
# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected.
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

FILE       = sous-vide
DEVICE     = atmega32
CLOCK      = 1000000
PROGRAMMER = -c dragon_isp -P usb
FUSES      = -U lfuse:w:0xe1:m -U hfuse:w:0xd9:m
# fuses set to disable jtag

# Typically, no changes are required beyond this point.

OBJECTS    = $(FILE).o lcd.o

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE) -x devcode=0x57
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -I./

# symbolic targets:
all:	$(FILE).hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:$(FILE).hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID $(FILE).hex

clean:
	rm -f $(FILE).hex $(FILE).elf $(OBJECTS)

# file targets:
$(FILE).elf: $(OBJECTS)
	$(COMPILE) -o $(FILE).elf $(OBJECTS)

$(FILE).hex: $(FILE).elf
	rm -f $(FILE).hex
	avr-objcopy -j .text -j .data -O ihex $(FILE).elf $(FILE).hex
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	$(FILE).elf
	avr-objdump -d $(FILE).elf

cpp:
	$(COMPILE) -E $(FILE).c lcd.c
