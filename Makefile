##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
# -Wdouble-promotion -fno-omit-frame-pointer

GIT_VERSION := git branch 
GIT_VERSION += $(shell git rev-parse --abbrev-ref HEAD) : sha 
GIT_VERSION += $(shell git describe --abbrev=4 --dirty --always --tags)

#$(info $$GIT_VERSION is [${GIT_VERSION}])
OPT_DMAX := DMAX
OPT_DSPEED := DSPEED
OPT_DNT := DNT
OPT_DEBUG := DEBUG
OPT_SPEED := SPEED
OPT_SIZE := SIZE

ifeq 	($(BUILD),)
	BUILD := $(OPT_DEBUG)
endif

SWDIO_DETECTION := 1
GCCVERSIONGTEQ10 := $(shell expr `arm-none-eabi-gcc -dumpversion | cut -f1 -d.` \>= 10)
GCCVERSIONGTEQ11 := $(shell expr `arm-none-eabi-gcc -dumpversion | cut -f1 -d.` \>= 11)
GCC_DIAG =  -Werror -Wno-error=unused-variable -Wno-error=format \
	    -Wno-error=cpp -Wno-error=type-limits \
            -Wno-error=unused-function \
            -Wunused -Wpointer-arith \
            -Werror=sign-compare \
            -Wshadow -Wparentheses -fmax-errors=5 \
            -ftrack-macro-expansion=2 -Wno-error=strict-overflow -Wstrict-overflow=2 \
            -Wduplicated-branches -Wdangling-else \
	    -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches \
            -Wlogical-op -Wdouble-promotion \
            -Wformat-overflow=2

G++_DIAG =   -Wnon-virtual-dtor -Woverloaded-virtual   \
	     -Wnull-dereference

ifeq "$(GCCVERSIONGTEQ10)" "1"
    GCC_DIAG += -Wno-error=volatile 
    G++_DIAG += -Wno-volatile -Wno-error=deprecated-declarations
endif

ifeq "$(GCCVERSIONGTEQ11)" "1"
    G++_DIAG += -Wno-deprecated-enum-enum-conversion -Wno-psabi
endif

UNUSED_DIAGS = -Wcast-align -Wsign-conversion -Wconversion

ifeq ($(BUILD),$(OPT_DEBUG)) 
  USE_OPT =  -Og -ggdb3  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	    $(GCC_DIAG)
  PROJECT = smartprobe_debug
  USE_PROCESS_STACKSIZE = 0x3800
# add -DTRACE to UDEFS for shell mode
  UDEFS = -DCH_DBG_STATISTICS=1 -DCH_DBG_SYSTEM_STATE_CHECK=1 \
          -DCH_DBG_ENABLE_CHECKS=1 -DCH_DBG_ENABLE_ASSERTS=1 -DTLSF_DEBUG=1 -D_DEBUG=1 -DTRACE
  USE_LTO = no
else ifeq ($(BUILD),$(OPT_DMAX)) 
  USE_OPT =  -O0 -g -ggdb3  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	    $(GCC_DIAG)
  PROJECT = smartprobe_debug
  USE_PROCESS_STACKSIZE = 0x4F00
  UDEFS = -DCH_DBG_STATISTICS=1 -DCH_DBG_SYSTEM_STATE_CHECK=1 \
          -DCH_DBG_ENABLE_CHECKS=1 -DCH_DBG_ENABLE_ASSERTS=1 -DTLSF_DEBUG=1 -D_DEBUG=1 -DTRACE
  USE_LTO = no
else ifeq ($(BUILD),$(OPT_DSPEED)) 
  USE_OPT =  -Ofast -flto=8 -fno-fast-math -g -ggdb3  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	    $(GCC_DIAG)
  PROJECT = smartprobe_debug
  USE_PROCESS_STACKSIZE = 0x3800
  UDEFS = -DCH_DBG_STATISTICS=1 -DCH_DBG_SYSTEM_STATE_CHECK=1 \
          -DCH_DBG_ENABLE_CHECKS=1 -DCH_DBG_ENABLE_ASSERTS=1 -DTLSF_DEBUG=1 -D_DEBUG=1 -DTRACE
  USE_LTO = yes
else ifeq ($(BUILD),$(OPT_DNT)) 
  USE_OPT =  -Og -ggdb3  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	    $(GCC_DIAG)
  PROJECT = smartprobe_debug
  USE_PROCESS_STACKSIZE = 0x2F00
  UDEFS = -DCH_DBG_STATISTICS=1 -DCH_DBG_SYSTEM_STATE_CHECK=1 \
          -DCH_DBG_ENABLE_CHECKS=1 -DCH_DBG_ENABLE_ASSERTS=1 -DTLSF_DEBUG=1 -D_DEBUG=1
  USE_LTO = no
else ifeq ($(BUILD),$(OPT_SPEED)) 
  USE_OPT =  -Ofast -fno-fast-math -flto=8  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	     $(GCC_DIAG)
  PROJECT = smartprobe_speed
  USE_PROCESS_STACKSIZE = 0x3800
  UDEFS = -DCH_DBG_STATISTICS=1 -DCH_DBG_SYSTEM_STATE_CHECK=0 -DCH_DBG_ENABLE_CHECKS=0 \
        -DCH_DBG_ENABLE_ASSERTS=0
  USE_LTO = yes
else ifeq ($(BUILD),$(OPT_SIZE)) 
  USE_OPT =  -Os  -flto=8  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
            --specs=nano.specs \
	     $(GCC_DIAG)
  PROJECT = smartprobe_size
  USE_PROCESS_STACKSIZE = 0x3800
  UDEFS = -DCH_DBG_STATISTICS=1 -DCH_DBG_SYSTEM_STATE_CHECK=0 -DCH_DBG_ENABLE_CHECKS=0 \
        -DCH_DBG_ENABLE_ASSERTS=0
  USE_LTO = yes
else
  $(error BUILD option ${BUILD} not recognised)
endif



# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = -std=gnu11   
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -std=gnu++23 -fno-rtti -fno-exceptions -fno-threadsafe-statics $(G++_DIAG)
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = --no-warn-rwx-segments
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
# 0x2F00
# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x600
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = hard
endif

# FPU-related options.
ifeq ($(USE_FPU_OPT),)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv5-d16
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here

# Target settings.
MCU  = cortex-m7

# Imported source files and paths

#CHIBIOS    := ./ext/chibios_svn_mirror
CHIBIOS    := /home/alex/DEV/STM32/CHIBIOS/ChibiOS_21.11_stable
CONFDIR    := ./cfg
BUILDDIR   := ./build
DEPDIR     := ./.dep
#VARIOUS    := ext/chibios_enac_various_common
VARIOUS    := /home/alex/DEV/STM32/CHIBIOS/COMMON/various
USBD_LIB   := $(VARIOUS)/Chibios-USB-Devices
TOOLDIR    := $(VARIOUS)/TOOLS
EIGEN_LIB  := ext/eigen
ETL_LIB    := ext/etl/include
FROZEN_LIB := ext/frozen/include
CTRE_LIB   := ext/compile-time-regular-expressions/single-header


EXTLIB = ext
PPRZ_MATH = $(VARIOUS)/paparazzi/math
# Licensing files.
include $(CHIBIOS)/os/license/license.mk
# Startup files.
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f7xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F7xx/platform.mk
include cfg/board.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMv7-M/compilers/GCC/mk/port.mk
# Auto-build files in ./source recursively.
include $(CHIBIOS)/tools/mk/autobuild.mk
# Other files (optional).
include $(CHIBIOS)/os/various/fatfs_bindings/fatfs.mk
include $(VARIOUS)/tlsf_bku/tlsf.mk
include $(EXTLIB)/STMems.mk


# Define linker script file here
LDSCRIPT= ${STARTUPLD}/STM32F76xxI.ld


# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC) \
       $(TLSFSRC) \
       $(STMEMSLPS33HWSRC) \
       $(CHIBIOS)/os/various/syscalls.c \
       $(VARIOUS)/stdutil.c \
       $(VARIOUS)/msg_queue.c \
       $(VARIOUS)/sdio.c \
       $(VARIOUS)/printf.c \
       $(VARIOUS)/microrl/microrlShell.c \
       $(VARIOUS)/microrl/microrl.c \
       $(VARIOUS)/i2cMaster.c \
       $(VARIOUS)/spiPeriphICM20600.c \
       $(VARIOUS)/rtcAccess.c \
       $(VARIOUS)/nmeaFrame.c \
       $(VARIOUS)/hal_stm32_dma.c \
       $(EXTLIB)/fnmatch.c \
       $(PPRZ_MATH)/pprz_geodetic_float.c \
       $(USBD_LIB)/mass_storage/usb_msd.c



# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC) \
         $(VARIOUS)/sdLiteLog.cpp


# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMXSRC = $(STARTUPASM) $(PORTASM) $(OSALASM)

INCDIR = $(CONFDIR) $(ALLINC) $(TLSFINC) \
         $(CHIBIOS)/os/various $(VARIOUS) $(VARIOUS_INCL) \
         $(STMEMSLPS33HWDIR) \
         $(CTRE_LIB) $(FROZEN_LIB) $(EIGEN_LIB) $(ETL_LIB)\
	 $(USBD_LIB)/mass_storage $(PPRZ_MATH)  $(EXTLIB)

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m7

#TRGT = arm-elf-
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc -pipe
CPPC = $(TRGT)g++ -pipe
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
#LD   = $(TRGT)gcc
LD   = $(TRGT)g++ -pipe
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp -pipe
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS += -DCTRE_STRING_IS_UTF8=1 -DFROZEN_NO_EXCEPTIONS=1 -DGIT_VERSION="$(GIT_VERSION)"

UDEFS += -DSWDIO_DETECTION=$(SWDIO_DETECTION)

# Define ASM defines here
UADEFS = $(UDEFS)



# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = -lstdc++ -lm

#
# End of user defines
##############################################################################

##############################################################################
# Common rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk
$(OBJS): $(CONFDIR)/board.h


$(CONFDIR)/board.h: $(CONFDIR)/board.cfg
	$(TOOLDIR)/boardGen.pl --no-adcp-in --no-pp-line $<  $@ || \
        cp -v cfg/board_template.h cfg/board.h


dfuflash: all
	@echo write $(BUILDDIR)/$(PROJECT).bin to flash memory
	dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000:leave $(BUILDDIR)/$(PROJECT).bin
	@echo Done

stflash: all
	@echo write $(BUILDDIR)/$(PROJECT).bin to flash memory
	st-flash write  $(BUILDDIR)/$(PROJECT).bin 0x08000000
	@echo Done

oneflash: all
	@echo write $(BUILDDIR)/$(PROJECT).elf to flash memory
	$(TOOLDIR)/bmpflash  $(BUILDDIR)/$(PROJECT).elf
	@echo Done

flash: all
	@echo write $(BUILDDIR)/$(PROJECT).elf to flash memory
	$(TOOLDIR)/bmpflash  $(BUILDDIR)/$(PROJECT).elf
ifeq ($(SWDIO_DETECTION),1)
	sleep 1
	$(TOOLDIR)/bmpflash  $(BUILDDIR)/$(PROJECT).elf	
endif
	@echo Done


.PHONY: force
compiler_flags: force
	echo '$(GIT_VERSION)' | cmp -s - $@ || echo '$(GIT_VERSION)' > $@
