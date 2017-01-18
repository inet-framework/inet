.PHONY: all clean cleanall makefiles makefiles-so makefiles-lib makefiles-exe checkmakefiles doxy doc

all: checkmakefiles src/inet/features.h 
	cd src && $(MAKE) all

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	@cd src && $(MAKE) MODE=release clean
	@cd src && $(MAKE) MODE=debug clean
	@rm -f src/Makefile src/inet/features.h
	@cd tutorials && $(MAKE) clean && rm -rf doc/tutorials

MAKEMAKE_OPTIONS := -f --deep -o INET -O out --no-deep-includes -I.

makefiles: src/inet/features.h makefiles-so

makefiles-so:
	@FEATURE_OPTIONS=$$(./inet_featuretool options -f -l -c) && cd src && opp_makemake --make-so $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-lib:
	@FEATURE_OPTIONS=$$(./inet_featuretool options -f -l -c) && cd src && opp_makemake --make-lib $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-exe:
	@FEATURE_OPTIONS=$$(./inet_featuretool options -f -l -c) && cd src && opp_makemake $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

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
	@./inet_featuretool defines >src/inet/features.h

doc:
	cd tutorials && $(MAKE) && mkdir -p ../doc/tutorials/wireless && cp -r wireless/html/* ../doc/tutorials/wireless
	cd doc/src/tcp && $(MAKE)
	cd doc/src/manual && $(MAKE)
	doxygen doxy.cfg
