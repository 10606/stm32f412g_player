
#######################################
# source paths
root = ../../..

utilities = $(root)/Utilities
fonts = $(utilities)/Fonts

BSP_src = $(root)/stm32/BSP

libs = $(root)/libs/STM32CubeF4
STM32_drivers = $(libs)/Drivers
STM32_middleware = $(libs)/Middlewares
CMSIS = $(STM32_drivers)/CMSIS
HAL_driver = $(STM32_drivers)/STM32F4xx_HAL_Driver/Src
USB_driver = $(STM32_middleware)/ST/STM32_USB_Device_Library

FAT32_driver = $(root)/FAT32_driver_char11/src
LCD_display = $(root)/LCD_display
MAD = Src/audio/mp3



######################################
# external sources

# HAL driver
HAL_driver_sources = \
	$(HAL_driver)/stm32f4xx_hal_pcd.c \
	$(HAL_driver)/stm32f4xx_hal_pcd_ex.c \
	$(HAL_driver)/stm32f4xx_hal_dfsdm.c \
	$(HAL_driver)/stm32f4xx_hal.c \
	$(HAL_driver)/stm32f4xx_hal_hcd.c \
	$(HAL_driver)/stm32f4xx_ll_usb.c \
	$(HAL_driver)/stm32f4xx_hal_rcc.c \
	$(HAL_driver)/stm32f4xx_hal_rcc_ex.c \
	$(HAL_driver)/stm32f4xx_hal_flash.c \
	$(HAL_driver)/stm32f4xx_hal_flash_ex.c \
	$(HAL_driver)/stm32f4xx_hal_flash_ramfunc.c \
	$(HAL_driver)/stm32f4xx_hal_gpio.c \
	$(HAL_driver)/stm32f4xx_hal_dma_ex.c \
	$(HAL_driver)/stm32f4xx_hal_dma.c \
	$(HAL_driver)/stm32f4xx_hal_pwr.c \
	$(HAL_driver)/stm32f4xx_hal_pwr_ex.c \
	$(HAL_driver)/stm32f4xx_hal_cortex.c \
	$(HAL_driver)/stm32f4xx_hal_exti.c \
	$(HAL_driver)/stm32f4xx_ll_fsmc.c \
	$(HAL_driver)/stm32f4xx_hal_sram.c \
	$(HAL_driver)/stm32f4xx_hal_i2c.c \
	$(HAL_driver)/stm32f4xx_hal_i2c_ex.c \
	$(HAL_driver)/stm32f4xx_hal_i2s.c \
	$(HAL_driver)/stm32f4xx_hal_i2s_ex.c \
	$(HAL_driver)/stm32f4xx_hal_qspi.c \
	$(HAL_driver)/stm32f4xx_ll_sdmmc.c \
	$(HAL_driver)/stm32f4xx_hal_sd.c \
	$(HAL_driver)/stm32f4xx_hal_tim.c \
	$(HAL_driver)/stm32f4xx_hal_tim_ex.c \
	$(HAL_driver)/stm32f4xx_hal_uart.c

# MAD
MAD_sources = \
	$(MAD)/stream.c \
	$(MAD)/frame.c \
	$(MAD)/synth.c \
	$(MAD)/bit.c \
	$(MAD)/timer.c \
	$(MAD)/layer12.c \
	$(MAD)/layer3.c \
	$(MAD)/huffman.c

# BSP
BSP_sources = \
	$(BSP_src)/f412g_disco/stm32412g_discovery.c \
	$(BSP_src)/f412g_disco/stm32412g_discovery_sd.c \
	$(BSP_src)/f412g_disco/stm32412g_discovery_audio.c \
	$(BSP_src)/f412g_disco/stm32412g_discovery_ts.c \
	$(BSP_src)/Components/ls016b8uy/ls016b8uy.c \
	$(BSP_src)/Components/st7789h2/st7789h2.c \
	$(BSP_src)/Components/wm8994/wm8994.c \
	$(BSP_src)/Components/ft6x06/ft6x06.c

# FAT32
FAT32_sources = \
	$(FAT32_driver)/init/init.c \
	$(FAT32_driver)/load_FAT/load_FAT.c \
	$(FAT32_driver)/open/open.c \
	$(FAT32_driver)/read/read.c \
	$(FAT32_driver)/read_file_info/read_file_info.c  \
	$(FAT32_driver)/file_descriptor/file_descriptor.c

# fonts
fonts_sources = \
	$(fonts)/font24.c \
	$(fonts)/font20.c \
	$(fonts)/font16.c \
	$(fonts)/font12.c \
	$(fonts)/font8.c 

# USB lib
USB_lib_sources = \
	$(USB_driver)/Core/Src/usbd_core.c \
	$(USB_driver)/Core/Src/usbd_ctlreq.c \
	$(USB_driver)/Core/Src/usbd_ioreq.c \
	$(USB_driver)/Class/CDC/Src/usbd_cdc.c

# LCD
LCD_sources = \
	$(LCD_display)/display_string.c \
	$(LCD_display)/display_init.c \
	$(LCD_display)/st7789h2_driver.c

# external sources
external_sources =  \
	$(MAD_sources) \
	$(BSP_sources) \
	$(HAL_driver_sources) \
	$(FAT32_sources) \
	$(LCD_sources) \
	$(fonts_sources) \
	$(USB_lib_sources)


######################################
# local sources

# main
main_sources = \
	Src/main/main.c \
	Src/main/player.c \
	Src/main/stm32f4xx_it.c  \
	Src/main/system_stm32f4xx.c
	
# USB
USB_sources = \
	Src/usb/usb_command_process.c \
	Src/usb/usbd_cdc_if.c \
	Src/usb/usbd_conf.c \
	Src/usb/usbd_desc.c \
	Src/usb/usb_device.c \
	Src/usb/usb_send.c

# display
display_sources = \
	Src/display/display_playlist.c \
	Src/display/display_pl_list.c \
	Src/display/display_song.c \
	Src/display/display_error.c

# playlist
playlist_sources = \
	Src/playlist/light_playlist.c \
	Src/playlist/playlist.c \
	Src/playlist/playlist_common.c \
	Src/playlist/playlist_view.c \
	Src/pl_list/pl_list.c

# user input
user_input_sources = \
	Src/joystick/joystick.c \
	Src/touch/touchscreen.c \
	Src/touch/moving.c

# audio
audio_sources = \
	Src/audio/audio.c \
	Src/audio/mp3.c \
	Src/audio/id3.c

# local sources
local_sources =  \
	$(main_sources) \
	Src/sd_card/sd_card_operation.c  \
	Src/view/view.c \
	$(audio_sources) \
	$(display_sources) \
	$(playlist_sources) \
	$(user_input_sources) \
	$(USB_sources)




#######################################
# external includes
external_includes = \
	-I$(USB_driver)/Core/Inc \
	-I$(USB_driver)/Class/CDC/Inc \
	-I$(utilities) \
	-I$(BSP_src) \
	-I$(BSP_src)/f412g_disco \
	-I$(BSP_src)/Components \
	-I$(BSP_src)/Components\Common \
	-I$(BSP_src)/Components\st7789h2 \
	-I$(BSP_src)/Components\ls016b8uy \
	-I$(BSP_src)/Components/wm8994 \
	-I$(HAL_driver)/../Inc/ \
	-I$(CMSIS)/Device/ST/STM32F4xx/Include/ \
	-I$(CMSIS)/Include/ \
	-I$(FAT32_driver) \
	-I$(LCD_display)/ \
	-I$(fonts) 

#######################################
# local includes
local_includes = \
	-ISrc/main \
	-ISrc/joystick \
	-ISrc/playlist \
	-ISrc/view \
	-ISrc/pl_list \
	-ISrc/display \
	-ISrc/touch \
	-ISrc/usb \
	-ISrc/audio/mp3 \
	-ISrc/audio \
	-ISrc/util \
	-ISrc/sd_card



