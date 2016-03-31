.PHONY: all clean cleanall makefiles makefiles-so makefiles-lib makefiles-exe checkmakefiles doxy

all: checkmakefiles src/inet/features.h 
	cd src && $(MAKE) all

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	@cd src && $(MAKE) MODE=release clean
	@cd src && $(MAKE) MODE=debug clean
	@rm -f src/Makefile src/inet/features.h

MAKEMAKE_OPTIONS := -f --deep -o INET -O out -pINET --no-deep-includes -I.

makefiles: src/inet/features.h makefiles-so

makefiles-so:
	@FEATURE_OPTIONS=$$(./inet_featuretool options -f -l) && cd src && opp_makemake --make-so $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-lib:
	@FEATURE_OPTIONS=$$(./inet_featuretool options -f -l) && cd src && opp_makemake --make-lib $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-exe:
	@FEATURE_OPTIONS=$$(./inet_featuretool options -f -l) && cd src && opp_makemake $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '========================================================================'; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '========================================================================'; \
	echo; \
	exit 1; \
	fi

# generate an include file that contains all the WITH_FEATURE macros according to the current enablement of features
src/inet/features.h: $(wildcard .oppfeaturestate) .oppfeatures
	@chmod +x ./inet_featuretool
	@./inet_featuretool defines >src/inet/features.h

doxy:
	doxygen doxy.cfg

tcptut:
	cd doc/src/tcp && $(MAKE)
