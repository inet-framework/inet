SOURCES = \
	ospf_abr.c ospf_flood.c ospf_neighbor.c ospf_snmp.c \
	ospf_api.c ospf_ia.c ospf_network.c ospf_spf.c \
	ospf_apiserver.c ospf_interface.c ospf_nsm.c ospf_te.c \
	ospf_asbr.c ospf_ism.c ospf_opaque.c ospf_vty.c \
	ospf_ase.c ospf_lsa.c ospf_packet.c ospf_zebra.c \
	ospfd.c ospf_lsdb.c ospf_route.c ospf_dump.c \
	ospf_main.c ospf_routemap.c

HEADERS = \
	ospf_abr.h ospf_dump.h ospf_lsdb.h ospf_route.h \
	ospf_api.h ospf_flood.h ospf_neighbor.h ospf_snmp.h \
	ospf_apiserver.h ospf_ia.h ospf_network.h ospf_spf.h \
	ospf_asbr.h ospf_interface.h ospf_nsm.h ospf_te.h \
	ospf_ase.h ospf_ism.h ospf_opaque.h ospf_vty.h \
	ospfd.h ospf_lsa.h ospf_packet.h ospf_zebra.h

OBJECTS = \
	ospf_abr.o ospf_flood.o ospf_neighbor.o ospf_snmp.o \
	ospf_api.o ospf_ia.o ospf_network.o ospf_spf.o \
	ospf_apiserver.o ospf_interface.o ospf_nsm.o ospf_te.o \
	ospf_asbr.o ospf_ism.o ospf_opaque.o ospf_vty.o \
	ospf_ase.o ospf_lsa.o ospf_packet.o ospf_zebra.o \
	ospfd.o ospf_lsdb.o ospf_route.o ospf_dump.o \
	ospf_main.o ospf_routemap.o

INCLUDES = -I./ -I../ -I../lib/ -I../../../

DEFS = -DHAVE_CONFIG_H -DOSPFD
CFLAGS = $(INCLUDES) $(DEFS) -fPIC -ggdb -Wimplicit -fexceptions
CC = gcc

all: ospfd.a

ospfd.a: $(OBJECTS) $(HEADERS) $(SOURCES)
	ar rvs ospfd.a $(OBJECTS)

clean:
	rm -f *.o ospfd.a
