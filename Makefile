all: makefiles
	cd src && $(MAKE)

clean:
	cd src && $(MAKE) clean

cleanall:
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean

makefiles:
	cd src && opp_makemake -f --deep -lpcap -o inet
	@if [ "$$LANG" != "" -o "$$LANGUAGE" != "" ]; then \
	echo; \
	echo '==============================================================================='; \
	echo 'NOTE: if you experience linker errors associated with IPv6ExtensionHeader, try the following commands:'; \
	echo '  unset LANG; unset LANGUAGE'; \
	echo 'See http://www.omnetpp.org/pmwiki/index.php?n=Main.INETLinkerErrorIPv6ExtensionHeader for more info.'; \
	echo '==============================================================================='; \
	echo; \
	fi

doxy:
	doxygen doxy.cfg

tcptut:
	cd doc/src/tcp && $(MAKE)
