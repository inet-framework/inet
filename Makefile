all: checkmakefiles
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles:
	@export NSC_VERSION=`ls -d 3rdparty/nsc* 2>/dev/null | sed 's/^.*-//'`; \
	if [ "$$NSC_VERSION" != "" ]; then \
	export NSC_VERSION_DEF=-KNSC_VERSION=$$NSC_VERSION; \
	fi; \
	cd src && opp_makemake -f --deep --make-so -o inet -O out $$NSC_VERSION_DEF

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to genrate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

doxy:
	doxygen doxy.cfg

tcptut:
	cd doc/src/tcp && $(MAKE)
