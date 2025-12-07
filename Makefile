PROJECT_NAME     := blinky_pca10059_mbr
TARGETS          := nrf52840_xxaa
OUTPUT_DIRECTORY := _build
DFU_PACKAGE      := $(OUTPUT_DIRECTORY)/nrf52840_xxaa.dfu
DFU_PORT         ?= /dev/ttyACM0

SDK_ROOT ?= /home/user/devel/esl-nsdk
PROJ_DIR := .

$(OUTPUT_DIRECTORY)/nrf52840_xxaa.out: \
  LINKER_SCRIPT := pca10059/mbr/armgcc/blinky_gcc_nrf52.ld

# Source files common to all targets
SRC_FILES += \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
  $(SDK_ROOT)/components/boards/boards.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
  $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
  $(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_pwm.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_nvmc.c \
  $(PROJ_DIR)/main.c \
  $(PROJ_DIR)/src/button.c \
  $(PROJ_DIR)/src/hsv.c \
  $(PROJ_DIR)/src/pwm_leds.c \
  $(PROJ_DIR)/src/storage.c \
  $(PROJ_DIR)/src/usb_cli.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd.c \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_core.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_serial_num.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_string_desc.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_usbd.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c

# Include folders common to all targets
INC_FOLDERS += \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(PROJ_DIR) \
  $(PROJ_DIR)/include \
  $(SDK_ROOT)/components/softdevice/mbr/headers \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/libraries/util \
  config \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/ringbuf \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/libraries/memobj \
  $(SDK_ROOT)/external/fprintf \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/modules/nrfx/drivers/src/prs \
  $(SDK_ROOT)/components/libraries/usbd \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc \
  $(SDK_ROOT)/external/utf_converter \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  $(SDK_ROOT)/integration/nrfx/legacy \

# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT = -O3 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DBOARD_PCA10059
CFLAGS += -DBSP_DEFINES_ONLY
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DMBR_PRESENT
CFLAGS += -DNRF52840_XXAA
CFLAGS += -DNRFX_GPIOTE_ENABLED=1
CFLAGS += -DNRFX_GPIOTE_CONFIG_IRQ_PRIORITY=6
CFLAGS += -DNRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS=2
CFLAGS += -DNRFX_PWM_ENABLED=1
CFLAGS += -DNRFX_PWM0_ENABLED=1
CFLAGS += -DNRFX_PWM1_ENABLED=1
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY=6
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_OUT0_PIN=31
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_OUT1_PIN=31
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_OUT2_PIN=31
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_OUT3_PIN=31
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_BASE_CLOCK=4
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_COUNT_MODE=0
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_TOP_VALUE=1000
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_LOAD_MODE=0
CFLAGS += -DNRFX_PWM_DEFAULT_CONFIG_STEP_MODE=0
CFLAGS += -DGPIOTE_ENABLED=1
CFLAGS += -DGPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS=2
CFLAGS += -DNRFX_PRS_ENABLED=1
CFLAGS += -DNRFX_PRS_BOX_0_ENABLED=1
CFLAGS += -DNRFX_PRS_BOX_1_ENABLED=1
CFLAGS += -DNRFX_PRS_CONFIG_IRQ_PRIORITY=6
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums
CFLAGS += -DNRFX_NVMC_ENABLED=1
CFLAGS += -DAPP_USBD_ENABLED=1
CFLAGS += -DAPP_USBD_CDC_ACM_ENABLED=1
CFLAGS += -DNRFX_USBD_ENABLED=1
CFLAGS += -DNRFX_CLOCK_ENABLED=1
CFLAGS += -DNRF_CLOCK_ENABLED=1
CFLAGS += -DAPP_USBD_VID=0x1915
CFLAGS += -DAPP_USBD_PID=0x520F
CFLAGS += -DNRFX_CLOCK_CONFIG_IRQ_PRIORITY=6
CFLAGS += -DNRFX_CLOCK_CONFIG_LF_SRC=1
CFLAGS += -DNRFX_USBD_CONFIG_IRQ_PRIORITY=6
CFLAGS += -DNRFX_POWER_ENABLED=0
CFLAGS += -DNRFX_USBD_CONFIG_IRQ_PRIORITY=6
CFLAGS += -DUSBD_CONFIG_IRQ_PRIORITY=6
CFLAGS += -DUSBD_ENABLED=1
CFLAGS += -DAPP_USBD_CDC_ACM_ZLP_ON_EPSIZE_WRITE=1
CFLAGS += -DNRFX_USBD_CONFIG_DMASCHEDULER_ISO_BOOST=1
CFLAGS += -DNRFX_USBD_CONFIG_ISO_IN_ZLP=1
CFLAGS += -DAPP_USBD_CONFIG_EVENT_QUEUE_ENABLE=1
CFLAGS += -DAPP_USBD_CONFIG_EVENT_QUEUE_SIZE=32
CFLAGS += -DAPP_USBD_DEVICE_VER_MAJOR=1
CFLAGS += -DAPP_USBD_DEVICE_VER_MINOR=0
CFLAGS += -DAPP_USBD_DEVICE_VER_SUB=0
CFLAGS += -DAPP_USBD_STRING_ID_MANUFACTURER=1
CFLAGS += -DAPP_USBD_STRING_ID_PRODUCT=2
CFLAGS += -DAPP_USBD_STRING_ID_SERIAL=3
CFLAGS += -DAPP_USBD_STRING_ID_CONFIGURATION=4
CFLAGS += -DAPP_USBD_MANUFACTURER_STRING=\"Nordic\"
CFLAGS += -DAPP_USBD_PRODUCT_STRING=\"USB_CLI\"
CFLAGS += -DAPP_USBD_SERIAL_NUMBER_STRING=\"123456\"
CFLAGS += -DAPP_USBD_CONFIGURATION_STRING=\"Default\"
CFLAGS += -DAPP_USBD_STRINGS_LANGIDS=0x0409
CFLAGS += '-DAPP_USBD_STRINGS_MANUFACTURER=APP_USBD_STRING_DESC(APP_USBD_MANUFACTURER_STRING)'
CFLAGS += '-DAPP_USBD_STRINGS_PRODUCT=APP_USBD_STRING_DESC(APP_USBD_PRODUCT_STRING)'
CFLAGS += '-DAPP_USBD_STRINGS_CONFIGURATION=APP_USBD_STRING_DESC(APP_USBD_CONFIGURATION_STRING)'
CFLAGS += '-DAPP_USBD_STRINGS_SERIAL=APP_USBD_STRING_DESC(APP_USBD_SERIAL_NUMBER_STRING)'
CFLAGS += '-DAPP_USBD_STRING_SERIAL=APP_USBD_STRING_DESC(APP_USBD_SERIAL_NUMBER_STRING)'
CFLAGS += -DAPP_USBD_STRINGS_USER=""
CFLAGS += -DAPP_USBD_CONFIG_DESC_STRING_SIZE=31
CFLAGS += -DAPP_USBD_CONFIG_MAX_POWER=100
CFLAGS += -DAPP_USBD_CONFIG_SELF_POWERED=1

# C++ flags common to all targets
CXXFLAGS += $(OPT)

# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBOARD_PCA10059
ASMFLAGS += -DBSP_DEFINES_ONLY
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DMBR_PRESENT
ASMFLAGS += -DNRF52840_XXAA

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

nrf52840_xxaa: CFLAGS += -D__HEAP_SIZE=8192
nrf52840_xxaa: CFLAGS += -D__STACK_SIZE=8192
nrf52840_xxaa: ASMFLAGS += -D__HEAP_SIZE=8192
nrf52840_xxaa: ASMFLAGS += -D__STACK_SIZE=8192

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm

.PHONY: default help

# Default target - first one defined
default: nrf52840_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		nrf52840_xxaa
	@echo		flash      - flashing binary

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: dfu

dfu_package: $(DFU_PACKAGE)

$(DFU_PACKAGE): $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	@echo Creating DFU package: $(DFU_PACKAGE)
	nrfutil pkg generate \
	   --hw-version 52 \
	   --application-version 1 \
	   --sd-req 0x0,0x102 \
	   --sd-id 0x102 \
	   --application $< \
	   --softdevice $(SDK_ROOT)/components/softdevice/s113/hex/s113_nrf52_7.2.0_softdevice.hex $@

dfu: $(DFU_PACKAGE)
	@echo Performing DFU with generated package
	nrfutil dfu usb-serial -pkg $< -p $(DFU_PORT) -b 115200