SOURCES = \
        network.c pid_output.c getopt.c getopt1.c \
        checksum.c vector.c linklist.c vty.c command.c \
        sockunion.c prefix.c thread.c if.c memory.c buffer.c table.c hash.c \
        filter.c routemap.c distribute.c stream.c str.c log.c plist.c \
        zclient.c sockopt.c smux.c md5.c if_rmap.c keychain.c privs.c \
        sigevent.c pqueue.c jhash.c

HEADERS = \
	buffer.h command.h filter.h getopt.h hash.h if.h linklist.h log.h \
	memory.h network.h prefix.h routemap.h distribute.h sockunion.h \
	str.h stream.h table.h thread.h vector.h version.h vty.h zebra.h \
	plist.h zclient.h sockopt.h smux.h md5-gnu.h if_rmap.h keychain.h \
	privs.h sigevent.h pqueue.h jhash.h zassert.h

OBJECTS = \
        network.o pid_output.o getopt.o getopt1.o \
        checksum.o vector.o linklist.o vty.o command.o \
        sockunion.o prefix.o thread.o if.o memory.o buffer.o table.o hash.o \
        filter.o routemap.o distribute.o stream.o str.o log.o plist.o \
        zclient.o sockopt.o smux.o md5.o if_rmap.o keychain.o privs.o \
        sigevent.o pqueue.o jhash.o

INCLUDES = -I./ -I../ -I../../../

DEFS = -DHAVE_CONFIG_H
CFLAGS = $(INCLUDES) $(DEFS) -fPIC -ggdb -Wimplicit -fexceptions
CC = gcc

all: libzebra.a

libzebra.a: $(OBJECTS) $(HEADERS) $(SOURCES)
	ar rvs libzebra.a $(OBJECTS)

clean:
	rm -f *.o *.a
