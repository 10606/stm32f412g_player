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
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Wall -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
Src/main.c \
Src/audio.c \
Src/fatfs_storage.c \
Src/sd_diskio.c  \
Src/stm32f4xx_it.c  \
Src/system_stm32f4xx.c \
Src/display/display_playlist.c \
Src/display/display_pl_list.c \
Src/display/display_song.c \
Src/playlist/light_playlist.c \
Src/playlist/playlist.c \
Src/playlist/playlist_common.c \
Src/playlist/playlist_view.c \
Src/view/view.c \
Src/touch/touchscreen.c \
Src/touch/moving.c \
Src/pl_list/pl_list.c \
Src/usb/usb_command_process.c \
Src/usb/usbd_cdc_if.c \
Src/usb/usbd_conf.c \
Src/usb/usbd_desc.c \
Src/usb/usb_device.c \
../../BSP/f412g_disco/stm32412g_discovery.c \
../../BSP/f412g_disco/stm32412g_discovery_lcd.c \
../../BSP/f412g_disco/stm32412g_discovery_sd.c \
../../BSP/f412g_disco/stm32412g_discovery_audio.c \
../../BSP/f412g_disco/stm32412g_discovery_ts.c \
../../BSP/Components/ls016b8uy/ls016b8uy.c \
../../BSP/Components/st7789h2/st7789h2.c \
../../BSP/Components/wm8994/wm8994.c \
../../BSP/Components/ft6x06/ft6x06.c  \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dfsdm.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hcd.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fsmc.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s_ex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_qspi.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_sdmmc.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sd.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c  \
../../../FAT32_driver_char11/src/init/init.c \
../../../FAT32_driver_char11/src/load_FAT/load_FAT.c \
../../../FAT32_driver_char11/src/open/open.c \
../../../FAT32_driver_char11/src/read/read.c \
../../../FAT32_driver_char11/src/read_file_info/read_file_info.c  \
../../../FAT32_driver_char11/src/file_descriptor/file_descriptor.c \
../../../LCD_display/display_string.c \
../../../Utilities/Fonts/font24.c \
../../../Utilities/Fonts/font20.c \
../../../Utilities/Fonts/font16.c \
../../../Utilities/Fonts/font12.c \
../../../Utilities/Fonts/font8.c  \
../../STM32CubeF4/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
../../STM32CubeF4/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
../../STM32CubeF4/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
../../STM32CubeF4/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
#../../../libs/STM32CubeF4/Middlewares/Third_Party/FatFS/src/option/ccsbcs.c \
#../../BSP/f412g_disco/stm32412g_discovery_eeprom.c \
#../../BSP/f412g_disco/stm32412g_discovery_qspi.c \
#../../BSP/f412g_disco/stm32412g_discovery_ts.c \
#Src/stm32f4xx_it.c \
#Src/stm32f4xx_hal_msp.c \
#Src/system_stm32f4xx.c \
#Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_core.c \
#Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ctlreq.c \
#Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ioreq.c \
#Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_pipes.c \
#Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Src/usbh_cdc.c \
#Src/usb_host.c \
#Src/usbh_conf.c \
#Src/usbh_platform.c \
#../../../Utilities/Fonts/font16.c 
#../../BSP/f412g_disco/stm32412g_discovery_audio.c \

# ASM sources
ASM_SOURCES =  \
startup_stm32f412zx.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F412Zx


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-IInc \
-ISrc/playlist \
-ISrc/view \
-ISrc/pl_list \
-ISrc/display \
-ISrc/touch \
-ISrc/usb \
-I../../STM32CubeF4/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
-I../../STM32CubeF4/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
-I../../../Utilities \
-I../../BSP \
-I../../BSP/f412g_disco \
-I../../BSP/Components \
-I../../BSP/Components\Common \
-I../../BSP/Components\st7789h2 \
-I../../BSP/Components\ls016b8uy \
-I../../BSP/Components/wm8994 \
-I../../../libs/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Inc \
-I../../../libs/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Include/ \
-I../../../libs/STM32CubeF4/Drivers/CMSIS/Include/ \
-I../../../FAT32_driver_char11/src/ \
-I../../../LCD_display/ \
-I../../../Utilities/Fonts/ \
-I../../../Middlewares/Third_Party/FatFS/src
#-IDrivers/STM32F4xx_HAL_Driver/Inc \
#-IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy \
#-IMiddlewares/ST/STM32_USB_Host_Library/Core/Inc \
#-IMiddlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc \
#-IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
#-IDrivers/CMSIS/Include \



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

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
LIBS = -lc -lm -lnosys 
LIBDIR = 
#LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

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

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
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

# *** EOF ***
