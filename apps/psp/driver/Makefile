TARGET = pspdl_driver
OBJS = pspdl_driver_kernel.o

# Build as a kernel-mode PRX
BUILD_PRX = 1
# Attribute 0x1000 required for kernel loading (no main thread, just library)
MODINFO_FLAGS = 0x1006

# Link standard kernel stubs for loadmodule, event flags, etc.
LIBS = -lpspkernel



# Use the PRX exports/import tables generation tools
PRX_EXPORTS = pspdl_driver.exp

CFLAGS = -O2 -G0 -Wall -D_PSP_FW_VERSION=660
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSPSDK = $(shell psp-config --pspsdk-path)

# Custom targets to run after standard build.mak builds the PRX
all: pspdl_driver.prx libpspdl_driver.a

libpspdl_driver.a: pspdl_driver.exp
	psp-build-exports -s pspdl_driver.exp
	psp-gcc $(CFLAGS) -I. -I$(PSPSDK)/include -c pspdl_driver.S -o pspdl_driver_stub.o
	psp-ar q $@ pspdl_driver_stub.o
	psp-ranlib $@

clean-custom:
	rm -f pspdl_driver.S pspdl_driver_stub.o libpspdl_driver.a

# Include standard PSPSDK build rules
include $(PSPSDK)/lib/build.mak

clean: clean-custom

