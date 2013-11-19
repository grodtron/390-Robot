# Microcontroller for gcc's -mmcu option
MCU     = atmega8
# Optimization level for gcc
OPT_LVL = 1

# Warning flags (what did you expect)
WARNINGS= -Wall -Wextra -Werror

# C Compiler
CC      = avr-gcc
CCFLAGS = -c -mmcu=$(MCU) -O$(OPT_LVL) $(WARNINGS) -std=gnu99

# Linker
LD      = avr-gcc
LDFLAGS = -mmcu=$(MCU) -O$(OPT_LVL) $(WARNINGS)

# Gotta call it something...
PROJ    = robot
# Source files that we will use
SRCS    := driver.c motors.c event_queue.c adc.c leds.c

# Stuff related to generating the actual hex format
OBJCOPY = avr-objcopy
# intel hex
HEX_FMT = ihex

# AVR, dude
AVRDUDE            = avrdude
# USBTiny is the ISP
AVRDUDE_PROGRAMMER = usbtiny
# atmega 8 is the chip
AVRDUDE_MCU        = m8
# intel hex format
AVRDUDE_WRITE_FMT  = i

ELF_TRG = $(PROJ).elf
HEX_TRG = $(PROJ).hex
ALL_TRG = $(HEX_TRG)


OBJ_DIR = .obj
MAKE_OBJ_DIR = mkdir -p $(OBJ_DIR)
SRC_DIR =  src

# Generate exact dependencies using a smart method that I found online.
# 
# The basic idea is that we don't need to know the dependencies until
# after the first time we build a file. If the file itself changes, then
# obviously we will have to recalculate its dependencies, but %.c is always
# a dependency of %.o, so in that case it would be recompiled anyways
#
# TODO - find the link I got this method from
DEP_DIR     = .dep
MAKE_DEPEND = mkdir -p $(DEP_DIR); \
				  $(CC) -MM -MT $(OBJ_DIR)/$*.o $(CCFLAGS) $< -o $(DEP_DIR)/$*.d; \

# Phony targets
.PHONY: all clean writeflash

# Make our dep_dir and our hex file
all: $(ALL_TRG)


# get rid of all the shit we created
clean:
	rm -rf $(DEP_DIR)
	rm -rf $(OBJ_DIR)
	rm -f $(PROJ).elf
	rm -f $(PROJ).hex

# Here's how we make our dep dir. (Not alll that complicated)
$(DEP_DIR):
	mkdir -p $(DEP_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Here is where all the .dep file are #include'd
-include $(SRCS:%.c=$(DEP_DIR)/%.d)

# For each c file, we compile it to an o file, and then make a
# dependency file for it, as explained above
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MAKE_OBJ_DIR)
	$(CC) $(CCFLAGS) $< -o $@
	@$(MAKE_DEPEND) 

# Our ELF file requires all our o files, and is fairly simple to make
$(ELF_TRG): $(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))
	$(LD) $(LDFLAGS) $^ -o $@

# Our hex file comes from our elf file
%.hex: %.elf
	$(OBJCOPY) -j .rodata -j .text -j .data -O $(HEX_FMT) $< $@

# Phony target to write our hex file to the MCU
writeflash: $(HEX_TRG)
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMER) -p $(AVRDUDE_MCU) -e -U flash:w:$(PROJ).hex:$(AVRDUDE_WRITE_FMT)

