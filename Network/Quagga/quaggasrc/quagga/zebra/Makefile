
SOURCES = \
	zserv.c main.c interface.c connected.c zebra_rib.c \
	redistribute.c debug.c rtadv.c zebra_snmp.c zebra_vty.c \
	irdp_main.c irdp_interface.c irdp_packet.c router-id.c \
	\
	rt_netlink.c rtread_netlink.c \
	ipforward_proc.c \
	if_proc.c if_netlink.c \
	ioctl.c

HEADERS = \
	connected.h ioctl.h rib.h rt.h zserv.h redistribute.h debug.h rtadv.h \
	interface.h ipforward.h irdp.h router-id.h

OBJECTS = \
	zserv.o main.o interface.o connected.o zebra_rib.o \
	redistribute.o debug.o rtadv.o zebra_snmp.o zebra_vty.o \
	irdp_main.o irdp_interface.o irdp_packet.o router-id.o \
	\
	rt_netlink.o rtread_netlink.o \
	ipforward_proc.o \
	if_proc.o if_netlink.o \
	ioctl.o

INCLUDES = -I./ -I../ -I../lib/ -I../../../

DEFS = -DHAVE_CONFIG_H -DZEBRA
CFLAGS = $(INCLUDES) $(DEFS) -fPIC -ggdb -Wimplicit -fexceptions
CC = gcc

all: zebra.a

zebra.a:  $(OBJECTS) $(HEADERS) $(SOURCES)
	ar rvs zebra.a $(OBJECTS)

clean:
	rm -f *.o *.a
