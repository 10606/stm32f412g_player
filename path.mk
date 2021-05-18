
#######################################
# source paths
root = ../../..

BSP_src = $(root)/stm32/BSP

libs = $(root)/libs/STM32CubeF4
STM32_drivers = $(libs)/Drivers
CMSIS = $(STM32_drivers)/CMSIS
HAL_driver = $(STM32_drivers)/STM32F4xx_HAL_Driver/Src
USB_driver = $(libs)/Middlewares/ST/STM32_USB_Device_Library

FAT32_driver = $(root)/FAT32_driver_char11/src
LCD_display = $(root)/LCD_display
fonts = $(LCD_display)/fonts
TS_touch_screen = $(root)/TS_touch_screen
MAD = src/audio/mp3



######################################
# external sources

# HAL driver
HAL_driver_sources_ = \
	stm32f4xx_hal.c \
	stm32f4xx_ll_fsmc.c \
	stm32f4xx_ll_sdmmc.c \
	stm32f4xx_ll_usb.c \
	stm32f4xx_hal_cortex.c \
	stm32f4xx_hal_gpio.c \
	stm32f4xx_hal_rcc.c \
	stm32f4xx_hal_rcc_ex.c \
	stm32f4xx_hal_dma.c \
	stm32f4xx_hal_dma_ex.c \
	stm32f4xx_hal_i2c.c \
	stm32f4xx_hal_i2c_ex.c \
	stm32f4xx_hal_i2s.c \
	stm32f4xx_hal_i2s_ex.c \
	stm32f4xx_hal_pcd.c \
	stm32f4xx_hal_pcd_ex.c \
	stm32f4xx_hal_tim.c \
	stm32f4xx_hal_tim_ex.c \
	stm32f4xx_hal_sd.c \
	stm32f4xx_hal_sram.c \
	stm32f4xx_hal_uart.c 
HAL_driver_sources = $(addprefix $(HAL_driver)/, $(HAL_driver_sources_))

# MAD
MAD_sources_ = \
	stream.c \
	frame.c \
	synth.c \
	bit.c \
	timer.c \
	layer12.c \
	layer3.c \
	huffman.c
MAD_sources = $(addprefix $(MAD)/, $(MAD_sources_))

# BSP
BSP_sources_ = \
	f412g_disco/stm32412g_discovery.c \
	f412g_disco/stm32412g_discovery_sd.c \
	f412g_disco/stm32412g_discovery_audio.c \
	Components/wm8994/wm8994.c \
	Components/ft6x06/ft6x06.c
BSP_sources = $(addprefix $(BSP_src)/, $(BSP_sources_))

# FAT32
FAT32_sources_ = \
	fat_info/fat_info.cpp \
	fat_info/load_FAT.cpp \
	open/open.cpp \
	file_descriptor/read.cpp \
	read_file_info/read_file_info.cpp 
FAT32_sources = $(addprefix $(FAT32_driver)/, $(FAT32_sources_))

# fonts
fonts_sources_ = \
	font_24.c \
	font_22.c \
	font_20.c \
	font_18.c \
	font_16.c \
	font_14.c \
	font_12.c
fonts_sources = $(addprefix $(fonts)/, $(fonts_sources_))

# USB lib
USB_lib_sources_ = \
	Core/Src/usbd_core.c \
	Core/Src/usbd_ctlreq.c \
	Core/Src/usbd_ioreq.c \
	Class/CDC/Src/usbd_cdc.c
USB_lib_sources = $(addprefix $(USB_driver)/, $(USB_lib_sources_))

# LCD
LCD_sources_ = \
	display_string.c \
	display_init.c \
	st7789h2_driver.c \
	display_init_st7789h2.c
LCD_sources = $(addprefix $(LCD_display)/, $(LCD_sources_))

# touch screen
TS_sources = \
	$(TS_touch_screen)/ts_touchscreen.cpp

# external sources
external_sources =  \
	$(MAD_sources) \
	$(BSP_sources) \
	$(HAL_driver_sources) \
	$(FAT32_sources) \
	$(LCD_sources) \
	$(TS_sources) \
	$(fonts_sources) \
	$(USB_lib_sources)


######################################
# local sources

# main
main_sources_ = \
	main.cpp \
	init.cpp \
	player.cpp \
	stm32f4xx_it.cpp  \
	system_stm32f4xx.c
main_sources = $(addprefix src/main/, $(main_sources_))
	
# USB
USB_sources_ = \
	usb_command_process.cpp \
	usbd_cdc_if.c \
	usbd_conf.c \
	usbd_desc.c \
	usb_device.c \
	usb_send.cpp
USB_sources = $(addprefix src/usb/, $(USB_sources_))

# display
display_sources_ = \
	display_playlist.cpp \
	display_pl_list.cpp \
	display_song.cpp \
	display_error.cpp \
	display_picture.cpp \
	display_common.cpp
display_sources = $(addprefix src/display/, $(display_sources_))

# playlist
playlist_sources_ = \
	light_playlist.cpp \
	playlist.cpp \
	playlist_view.cpp
playlist_sources = \
	$(addprefix src/playlist/, $(playlist_sources_)) \
	src/pl_list/pl_list.cpp

# user input
user_input_sources = \
	src/joystick/joystick.cpp \
	src/touch/touchscreen.cpp \
	src/touch/moving.cpp

# audio
audio_sources_ = \
	audio.cpp \
	mp3.cpp \
	id3.cpp
audio_sources = $(addprefix src/audio/, $(audio_sources_))

# local sources
local_sources =  \
	$(main_sources) \
	src/sd_card/sd_card_operation.cpp  \
	src/util/util.cpp \
	src/view/view.cpp \
	src/view/process_view.cpp \
	$(audio_sources) \
	$(playlist_sources) \
	$(user_input_sources) \
	$(USB_sources) \
	$(display_sources)




#######################################
# external includes
external_includes = \
	-I$(USB_driver)/Core/Inc \
	-I$(USB_driver)/Class/CDC/Inc \
	-I$(BSP_src)/f412g_disco \
	-I$(BSP_src)/Components \
	-I$(HAL_driver)/../Inc/ \
	-I$(CMSIS)/Device/ST/STM32F4xx/Include/ \
	-I$(CMSIS)/Include/ \
	-I$(FAT32_driver) \
	-I$(LCD_display)/ \
	-I$(TS_touch_screen)/ \
	-I$(fonts) 

#######################################
# local includes
local_includes_ = \
	main \
	joystick \
	playlist \
	view \
	pl_list \
	display \
	touch \
	usb \
	audio/mp3 \
	audio \
	util \
	sd_card
local_includes = $(addprefix -Isrc/, $(local_includes_))



