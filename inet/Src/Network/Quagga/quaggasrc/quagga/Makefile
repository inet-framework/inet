include ../../../../inetconfig

INCLUDES = -I./ -I./lib -I../../

DEFS = -DHAVE_CONFIG_H
CFLAGS += $(INCLUDES) $(DEFS) -fPIC -Wimplicit -fexceptions

all: libdir ripddir zebradir ospfdir bgpddir globalvars.o

libdir:
	(cd lib; make)

ripddir:
	(cd ripd; make)

zebradir:
	(cd zebra; make)

ospfdir:
	(cd ospfd; make)

bgpddir:
	(cd bgpd; make)

clean:
	(cd lib; make clean)
	(cd ripd; make clean)
	(cd zebra; make clean)
	(cd ospfd; make clean)
	(cd bgpd; make clean)
	rm globalvars.o

