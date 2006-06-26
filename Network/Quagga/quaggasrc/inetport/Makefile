VERSION = 0.98.5
QUAGGA_VERSION = quagga-$(VERSION)
INETPORT_VERSION = inetport-$(VERSION)


all: extract prepatch1 prepatch2 doxy makepatch patch

extract:
	tar xvzf ../$(QUAGGA_VERSION).tar.gz -C ..
	( \
		cd ../$(QUAGGA_VERSION); \
		cat ../$(INETPORT_VERSION)/remove.lst | xargs rm -R; \
		cat ../$(INETPORT_VERSION)/staticmake.diff | patch -p1; \
	)
	
prepatch1:
	( \
		cd ../$(QUAGGA_VERSION); \
		for i in ../$(INETPORT_VERSION)/diffs/*.diff ; do 	patch -p0 < $$i ; done ; \
	)

prepatch2:
	( \
		cd ../$(QUAGGA_VERSION); \
		../$(INETPORT_VERSION)/dopatch.pl . -predoxy ; \
	)

doxy:
	( cd ../$(QUAGGA_VERSION); doxygen ../$(INETPORT_VERSION)/Doxyfile )

makepatch:
	xsltproc compact.xsl xml/index.xml
	xsltproc includes.xsl _source.xml
	xsltproc structs.xsl _source.xml
	xsltproc globals.xsl _source.xml
	xsltproc params.xsl _source.xml
	./whitelist.pl globalwhitelist.lst < 03_globals.patch.in > 03_globals.patch

patch:
	( \
		cd ../$(QUAGGA_VERSION); \
		../$(INETPORT_VERSION)/dopatch.pl . < ../$(INETPORT_VERSION)/00_syscalls.lst ; \
		../$(INETPORT_VERSION)/dopatch.pl . < ../$(INETPORT_VERSION)/02_struct.patch ;\
		../$(INETPORT_VERSION)/dopatch.pl . < ../$(INETPORT_VERSION)/03_globals.patch ; \
		mv globalvars.h.tmp globalvars.h ; \
		mv globalvars.c.tmp globalvars.c ; \
		mv globalvars_on.h.tmp globalvars_on.h ; \
		mv globalvars_off.h.tmp globalvars_off.h ; \
		cat ../$(INETPORT_VERSION)/02_struct.patch | grep -E "^S;" | ../$(INETPORT_VERSION)/dopatch.pl globalvars.h > /dev/null ; \
		cat ../$(INETPORT_VERSION)/02_struct.patch | grep -E "^S;" | ../$(INETPORT_VERSION)/dopatch.pl globalvars.c > /dev/null ; \
		../$(INETPORT_VERSION)/dopatch.pl . < ../$(INETPORT_VERSION)/04_params.patch ; \
		../$(INETPORT_VERSION)/dopatch.pl . < ../$(INETPORT_VERSION)/05_locals.lst ; \
		../$(INETPORT_VERSION)/dopatch.pl . < ../$(INETPORT_VERSION)/06_incl.patch ; \
	)

clean:
	rm -Rf *.patch *.patch.in _source.xml xml
