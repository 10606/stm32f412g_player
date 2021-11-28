##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [3.7.1] date: [Sun Jul 05 18:29:52 MSK 2020] 
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = f412g_2


######################################
# make sources
make_files = \
	Makefile \
	path.mk

######################################
# building variables
######################################
# debug build?
DEBUG = 0
# optimization
OPT = -Wall -flto


#######################################
# paths
#######################################
# build path
BUILD_DIR = build


include path.mk
######################################
# C sources
C_SOURCES =  \
	$(local_sources) \
	$(external_sources)

# ASM sources
ASM_SOURCES =  \
	startup_stm32f412zx.s


######################################
# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
	-DUSE_HAL_DRIVER \
	-DSTM32F412Zx \
	-DHAVE_CONFIG_H \
	-DFPM_DEFAULT \
	-DASO_INTERLEAVE2 \
	-DNDEBUG #\
	-DTS_AUTO_CALIBRATION_SUPPORTED=1


######################################
# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
	$(local_includes) \
	$(external_includes)


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
PP = $(GCC_PATH)/$(PREFIX)g++
CC = $(GCC_PATH)/$(PREFIX)gcc
LD = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
PP = $(PREFIX)g++
CC = $(PREFIX)gcc
LD = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
BSZ = echo -n "   " && du -b
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -march=armv7e-m

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Os -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -fno-exceptions

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F412ZGTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys -lstdc++
LIBDIR = 
#LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -flto -fno-exceptions -Os -fno-rtti

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
C__SOURCES = $(C_SOURCES:.cpp=.o)
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C__SOURCES:.c=.o))) 
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o))) 
vpath %.cpp $(sort $(dir $(C_SOURCES))) 
vpath %.c $(sort $(dir $(C_SOURCES))) 
vpath %.s $(sort $(dir $(ASM_SOURCES))) 

$(BUILD_DIR)/%.o: %.cpp $(make_files) | $(BUILD_DIR) 
	$(PP) -c $(CFLAGS) -Os -std=c++20 -fno-rtti -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/display_picture.o: display_picture.cpp $(make_files) | $(BUILD_DIR) 
	$(PP) -c $(CFLAGS) -O3 -std=c++20 -fno-rtti -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@
	
$(BUILD_DIR)/find_song.o: find_song.cpp $(make_files) | $(BUILD_DIR) 
	$(PP) -c $(CFLAGS) -O3 -std=c++20 -fno-rtti -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@
	
$(BUILD_DIR)/light_playlist.o: light_playlist.cpp $(make_files) | $(BUILD_DIR) 
	$(PP) -c $(CFLAGS) -O3 -std=c++20 -fno-rtti -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@
	
$(BUILD_DIR)/read.o: read.cpp $(make_files) | $(BUILD_DIR) 
	$(PP) -c $(CFLAGS) -O2 -std=c++20 -fno-rtti -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@
	
	
$(BUILD_DIR)/%.o: %.c $(make_files) | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Os -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
	
$(BUILD_DIR)/display_string.o: display_string.c $(make_files) | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -O3 -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
	

$(BUILD_DIR)/%.o: %.s $(make_files) | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(make_files) STM32F412ZGTx_FLASH.ld
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	$(BSZ) $@
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

LOADER = st-flash --reset write

load: $(BUILD_DIR)/$(TARGET).bin
	$(LOADER) $< 0x08000000
	
	
	
#######################################
# pictures (240 x 240)
#######################################
pictures/%.rgb565: pictures/%.png
	magick $< -depth 8 rgb:- | bmp2rgb565 $@
	
pictures/%.rgb565: pictures/%.jpg
	magick $< -depth 8 rgb:- | bmp2rgb565 $@
	
pictures/%.rgb565: pictures/%.jpeg
	magick $< -depth 8 rgb:- | bmp2rgb565 $@
	

pictures/%.ch565: pictures/%.rgb565 utilities/huffman_encode/huffman_encode
	utilities/huffman_encode/huffman_encode $< $@

ALL_PICTURES = 	pictures/song_0.ch565 \
		pictures/song_1.ch565 \
		pictures/song_2.ch565 \
		pictures/song_3.ch565 \
		pictures/song_4.ch565 \
		pictures/song_5.ch565 \
		pictures/song_6.ch565 \
		pictures/start_0.ch565 \
		pictures/start_1.ch565

pictures/all: $(ALL_PICTURES)
	cat $^ > $@

src/display/display_picture_offset.cpp: $(ALL_PICTURES) utilities/huffman_encode/offset_calculator
	utilities/huffman_encode/offset_calculator $@ 7 $(ALL_PICTURES)

utilities/huffman_encode/huffman_encode: utilities/huffman_encode/huffman_encode.cpp
	$(MAKE) -C utilities/huffman_encode/
	
utilities/huffman_encode/offset_calculator: utilities/huffman_encode/offset_calculator.cpp
	$(MAKE) -C utilities/huffman_encode/

load_pictures: pictures/all
	$(LOADER) $<  0x8040000

# *** EOF ***
