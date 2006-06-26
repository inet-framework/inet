SOURCES = \
	ripd.c rip_zebra.c rip_interface.c rip_debug.c rip_snmp.c \
	rip_routemap.c rip_peer.c rip_offset.c rip_main.c

HEADERS = \
	ripd.h rip_debug.h

OBJECTS = \
	ripd.o rip_zebra.o rip_interface.o rip_debug.o rip_snmp.o \
	rip_routemap.o rip_peer.o rip_offset.o rip_main.o

INCLUDES = -I./ -I../ -I../lib/ -I../../../

DEFS = -DHAVE_CONFIG_H -DRIPD
CFLAGS = $(INCLUDES) $(DEFS) -fPIC -ggdb -Wimplicit -fexceptions
CC = gcc

all: ripd.a

ripd.a: $(OBJECTS) $(HEADERS) $(SOURCES)
	ar rvs ripd.a $(OBJECTS)

clean:
	rm -f *.o ripd.a
