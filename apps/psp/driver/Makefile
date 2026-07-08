TARGET = pspdl_driver
OBJS = pspdl_driver_kernel.o

# Build as a kernel-mode PRX
BUILD_PRX = 1
MODINFO_FLAGS = 0x1006

USE_KERNEL_LIBS = 1
LIBS = -lpspusb_driver -lpspusbbus_driver

CFLAGS = -O2 -G0 -Wall -D_PSP_FW_VERSION=660
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak


