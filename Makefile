FEATURETOOL = opp_featuretool
FEATURES_H = src/inet/features.h
SELFDOC = tests/fingerprint/SelfDoc

.PHONY: all clean cleanall neddoc makefiles makefiles-so makefiles-lib makefiles-exe checkenvir checkmakefiles doxy doc submodule-init

all: src/Makefile $(FEATURES_H)
	@cd src && $(MAKE)

clean: src/Makefile
	@cd src && $(MAKE) clean

cleanall: src/Makefile
	@cd src && $(MAKE) MODE=release clean
	@cd src && $(MAKE) MODE=debug clean
	@rm -f src/Makefile $(FEATURES_H) $(SELFDOC).xml

INET_PROJ = $(shell inet_root)

MAKEMAKE_OPTIONS := -f --deep -o INET -O out -pINET -I.

src/Makefile: $(wildcard .oppfeaturestate) .oppfeatures
	$(MAKE) makefiles

$(FEATURES_H): $(wildcard .oppfeaturestate) .oppfeatures
	@$(FEATURETOOL) defines >$(FEATURES_H)

makefiles: makefiles-so

makefiles-so: checkenvir
	@FEATURE_OPTIONS=$$($(FEATURETOOL) options -f -l) && cd src && opp_makemake --make-so $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-lib: checkenvir
	@FEATURE_OPTIONS=$$($(FEATURETOOL) options -f -l) && cd src && opp_makemake --make-lib $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-exe: checkenvir
	@FEATURE_OPTIONS=$$($(FEATURETOOL) options -f -l) && cd src && opp_makemake $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

checkenvir:
	@if [ "$(INET_PROJ)" = "" ]; then \
	echo; \
	echo '==========================================================================='; \
	echo '<inet_root>/setenv is not sourced. Please change to the INET root directory'; \
	echo 'and type "source setenv" to initialize the environment!'; \
	echo '==========================================================================='; \
	echo; \
	exit 1; \
	fi

doc:
	@cd doc/src && $(MAKE)
	@doxygen doxy.cfg

ddoc:
	@cd doc/src && ./docker-make html && echo "===> file:$$(pwd)/_build/html/index.html"

$(SELFDOC).xml: $(SELFDOC).json
	@inet_selfdoc_json2xml <$< >$@

neddoc: $(SELFDOC).xml
	@opp_neddoc --verbose --no-automatic-hyperlinks -x "/*/examples,/*/tests,/*/showcases,/*/tutorials" -f $(SELFDOC).xml $(INET_PROJ)

