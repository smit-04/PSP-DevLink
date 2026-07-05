TARGET = HelloWorld
OBJS = main.o
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Hello World
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
