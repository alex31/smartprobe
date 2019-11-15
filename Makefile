##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
# -Wdouble-promotion -fno-omit-frame-pointer

DEBUG := 1
OPT_SPEED := 2
OPT_SIZE := 3

EXECMODE := $(DEBUG)
#EXECMODE := $(OPT_SPEED)
#EXECMODE := $(OPT_SIZE)


GCCVERSIONGTEQ7 := $(shell expr `arm-none-eabi-gcc -dumpversion | cut -f1 -d.` \>= 7)
GCC_DIAG =  -Werror -Wno-error=unused-variable -Wno-error=format \
            -Wno-error=cpp \
            -Wno-error=unused-function \
            -Wunused -Wpointer-arith \
            -Werror=sign-compare \
            -Wshadow -Wparentheses -fmax-errors=5 \
            -ftrack-macro-expansion=2 -Wno-error=strict-overflow -Wstrict-overflow=2

ifeq "$(GCCVERSIONGTEQ7)" "1"
    GCC_DIAG += -Wvla-larger-than=128 -Wduplicated-branches -Wdangling-else \
                -Wformat-overflow=2 -Wformat-truncation=2
endif



ifeq ($(EXECMODE),$(DEBUG)) 
  USE_OPT =  -O0  -ggdb3  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	    $(GCC_DIAG)
endif

ifeq ($(EXECMODE),$(OPT_SPEED)) 
    USE_OPT =  -Ofast -flto=4  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	     $(GCC_DIAG)
endif

ifeq ($(EXECMODE),$(OPT_SIZE)) 
    USE_OPT =  -Os  -flto=4  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
            --specs=nano.specs \
	     $(GCC_DIAG)
endif



# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = -std=gnu11   
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -std=c++17 -fno-rtti -fno-exceptions -fno-threadsafe-statics
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
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
#
# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
ifeq ($(EXECMODE),$(DEBUG))
  USE_PROCESS_STACKSIZE = 0x1A00
else
  USE_PROCESS_STACKSIZE = 0x600
endif
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
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
PROJECT = ch

# Target settings.
MCU  = cortex-m7

# Imported source files and paths
MY_DIRNAME=../../../ChibiOS_stable
ifneq "$(wildcard $(MY_DIRNAME) )" ""
   RELATIVE=../../..
else
  RELATIVE=../..
endif

CHIBIOS = $(RELATIVE)/ChibiOS_19.1_stable
CONFDIR  := ./cfg
BUILDDIR := ./build
DEPDIR   := ./.dep

STMSRC = $(RELATIVE)/COMMON/stm
VARIOUS = $(RELATIVE)/COMMON/various
USBD_LIB = $(VARIOUS)/Chibios-USB-Devices
ETL_LIB = ../../../../etl/include
FROZEN_LIB = ../../../../frozen/include
CTRE_LIB = ../../../.././compile-time-regular-expressions/single-header




# Licensing files.
include $(CHIBIOS)/os/license/license.mk
# Startup files.
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f7xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F7xx/platform.mk
include cfg/board.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
# Auto-build files in ./source recursively.
include $(CHIBIOS)/tools/mk/autobuild.mk
# Other files (optional).
include $(CHIBIOS)/os/various/fatfs_bindings/fatfs.mk
include $(VARIOUS)/tlsf_bku/tlsf.mk
include $(STMSRC)/STMems.mk


# Define linker script file here
LDSCRIPT= ${STARTUPLD}/STM32F76xxI.ld


# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC) \
       $(TLSFSRC) \
       $(STMEMSLPS33HWSRC) \
       $(CHIBIOS)/os/various/syscalls.c \
       $(VARIOUS)/stdutil.c \
       $(VARIOUS)/printf.c \
       $(VARIOUS)/microrl/microrlShell.c \
       $(VARIOUS)/microrl/microrl.c \
       $(VARIOUS)/sdLog.c \
       $(VARIOUS)/i2cMaster.c \
       $(VARIOUS)/spiPeriphICM20600.c \
       $(VARIOUS)/rtcAccess.c 



# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC)

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
         $(CTRE_LIB) $(ETL_LIB) $(FROZEN_LIB) 

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m7

#TRGT = arm-elf-
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
#LD   = $(TRGT)gcc
LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
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
ifeq ($(EXECMODE),$(DEBUG))
UDEFS = -DTRACE  -DCH_DBG_STATISTICS=1 -DCH_DBG_SYSTEM_STATE_CHECK=1 -DCH_DBG_ENABLE_CHECKS=1 \
        -DCH_DBG_ENABLE_ASSERTS=1
else
UDEFS = -DCH_DBG_STATISTICS=0 -DCH_DBG_SYSTEM_STATE_CHECK=0 -DCH_DBG_ENABLE_CHECKS=0 \
        -DCH_DBG_ENABLE_ASSERTS=0
endif

UDEFS += -DCTRE_STRING_IS_UTF8=1 -DFROZEN_NO_EXCEPTIONS=1

# Define ASM defines here
ifeq ($(EXECMODE),$(DEBUG))
UADEFS = -DCH_DBG_STATISTICS=1 -DCH_DBG_SYSTEM_STATE_CHECK=1 -DCH_DBG_ENABLE_CHECKS=1 \
         -DCH_DBG_ENABLE_ASSERTS=1
else
UADEFS = -DCH_DBG_STATISTICS=0 -DCH_DBG_SYSTEM_STATE_CHECK=0 -DCH_DBG_ENABLE_CHECKS=0 \
         -DCH_DBG_ENABLE_ASSERTS=0
endif


# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user defines
##############################################################################


RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk
$(OBJS): cfg/board.h

cfg/board.h: cfg/board.cfg Makefile
	boardGen.pl --no-pp-line --no-adcp-in	$<  $@

stflash:        all
	@echo write $(BUILDDIR)/$(PROJECT).bin to flash memory
	/usr/local/bin/st-flash write  $(BUILDDIR)/$(PROJECT).bin 0x08000000
	@echo Done

flash: all
	@echo write $(BUILDDIR)/$(PROJECT).bin to flash memory
	bmpflash  $(BUILDDIR)/$(PROJECT).elf
	@echo Done