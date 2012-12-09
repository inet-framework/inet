#
# Check operating system
#
UNAME := $(shell uname -o)
ifeq ($(UNAME),Msys)
OPP_LIBS =
else
OPP_LIBS = -loppcommon
endif

all: checkmakefiles
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles:
### To create a shared library
#	cd src && opp_makemake -f --deep --make-so -o inet -O out $$NSC_VERSION_DEF
##  for support of SQLite interfacing and custom result recorders
	cd src && opp_makemake -f --deep --make-so -o inet -O out $$NSC_VERSION_DEF -I$(OMNETPP_ROOT)/src/common -I$(OMNETPP_ROOT)/src/envir -I$(OMNETPP_ROOT)/include/platdep -I/usr/local/include -L/usr/local/lib $(OPP_LIBS) -lsqlite3
### To create a single executable
#	cd src && opp_makemake -f --deep -o inet -O out $$NSC_VERSION_DEF
##  for support of SQLite interfacing and custom result recorders
#	cd src && opp_makemake -f --deep -o inet -O out $$NSC_VERSION_DEF -I$(OMNETPP_ROOT)/src/common -I$(OMNETPP_ROOT)/src/envir -I$(OMNETPP_ROOT)/include/platdep $(OPP_LIBS) -lsqlite3

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

doxy:
	doxygen doxy.cfg

tcptut:
	cd doc/src/tcp && $(MAKE)
