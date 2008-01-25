include ../../../../../inetconfig

SOURCES = \
	bgpd.c bgp_fsm.c bgp_aspath.c bgp_community.c bgp_attr.c \
	bgp_debug.c bgp_route.c bgp_zebra.c bgp_open.c bgp_routemap.c \
	bgp_packet.c bgp_network.c bgp_filter.c bgp_regex.c bgp_clist.c \
	bgp_dump.c bgp_snmp.c bgp_ecommunity.c bgp_mplsvpn.c bgp_nexthop.c \
	bgp_damp.c bgp_table.c bgp_advertise.c bgp_vty.c bgp_main.c

HEADERS = \
	bgp_aspath.h bgp_attr.h bgp_community.h bgp_debug.h bgp_fsm.h \
	bgp_network.h bgp_open.h bgp_packet.h bgp_regex.h bgp_route.h \
	bgpd.h bgp_filter.h bgp_clist.h bgp_dump.h bgp_zebra.h \
	bgp_ecommunity.h bgp_mplsvpn.h bgp_nexthop.h bgp_damp.h bgp_table.h \
	bgp_advertise.h bgp_snmp.h bgp_vty.h

OBJECTS = \
	bgpd.o bgp_fsm.o bgp_aspath.o bgp_community.o bgp_attr.o \
	bgp_debug.o bgp_route.o bgp_zebra.o bgp_open.o bgp_routemap.o \
	bgp_packet.o bgp_network.o bgp_filter.o bgp_regex.o bgp_clist.o \
	bgp_dump.o bgp_snmp.o bgp_ecommunity.o bgp_mplsvpn.o bgp_nexthop.o \
	bgp_damp.o bgp_table.o bgp_advertise.o bgp_vty.o bgp_main.o

INCLUDES = -I./ -I../ -I../lib/ -I../../../

DEFS = -DHAVE_CONFIG_H -DBGPD
CFLAGS += $(INCLUDES) $(DEFS) -fPIC -Wimplicit -fexceptions

all: bgpd.a

bgpd.a: $(OBJECTS) $(HEADERS) $(SOURCES)
	ar rvs bgpd.a $(OBJECTS)

clean:
	rm -f *.o bgpd.a
