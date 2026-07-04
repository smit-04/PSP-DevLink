TARGET = PSPDevLink

OBJS = \
    src/main.o \
    src/transport_usb.o \
    ../../shared/protocol/src/packet.o

BUILD_PRX = 0

PSP_FW_VERSION = 660

LIBS += -lpspusbbus_driver -lpspusb_driver

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

INCDIR += ../../shared/protocol/include

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP DevLink

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak