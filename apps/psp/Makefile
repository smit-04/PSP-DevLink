TARGET = PSPDevLink

OBJS = \
    src/main.o \
    src/transport_usb.o \
    ../../shared/protocol/src/packet.o \
    ../../shared/protocol/src/payload.o \
    src/message_router.o \
    src/ui.o

# User-mode static ELF for CFW 6.60 PRO-B9

PSP_FW_VERSION = 660

LIBS += -lpsppower

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

INCDIR += ../../shared/protocol/include

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP DevLink

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak