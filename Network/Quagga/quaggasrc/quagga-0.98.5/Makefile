
INCLUDES = -I./ -I./lib -I../../

DEFS = -DHAVE_CONFIG_H
CFLAGS = $(INCLUDES) $(DEFS) -fPIC -ggdb -Wimplicit -fexceptions
CC = gcc

all: libdir ripddir zebradir ospfdir globalvars.o

libdir:
	cd lib && $(MAKE)

ripddir:
	cd ripd && $(MAKE)

zebradir:
	cd zebra && $(MAKE)

ospfdir:
	cd ospfd && $(MAKE)

clean:
	cd lib && $(MAKE) clean
	cd ripd && $(MAKE) clean
	cd zebra && $(MAKE) clean
	cd ospfd && $(MAKE) clean
	rm globalvars.o

