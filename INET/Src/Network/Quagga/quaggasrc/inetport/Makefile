QUAGGA_VERSION = `cat QUAGGA_VERSION`

all: extract prepatch1 prepatch2 doxy makepatch patch

extract:
	tar xvzf ../$(QUAGGA_VERSION).tar.gz -C ..
	( \
		cd ../$(QUAGGA_VERSION); \
		cat ../inetport/remove.lst | xargs rm -R; \
		cat ../inetport/staticmake.diff | patch -p1; \
	)
	
prepatch1:
	( \
		cd ../$(QUAGGA_VERSION); \
		for i in ../inetport/diffs/*.diff ; do 	patch -p0 < $$i ; done ; \
	)

prepatch2:
	( \
		cd ../$(QUAGGA_VERSION); \
		../inetport/dopatch.pl . -predoxy ; \
	)

doxy:
	( cd ../$(QUAGGA_VERSION); doxygen ../inetport/Doxyfile )

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
		../inetport/dopatch.pl . < ../inetport/00_syscalls.lst ; \
		../inetport/dopatch.pl . < ../inetport/02_struct.patch ; \
		../inetport/dopatch.pl . < ../inetport/02_struct-sigaction.lst  ; \
		../inetport/dopatch.pl . < ../inetport/03_globals.patch ; \
		mv globalvars.h.tmp globalvars.h ; \
		mv globalvars.c.tmp globalvars.c ; \
		mv globalvars_on.h.tmp globalvars_on.h ; \
		mv globalvars_off.h.tmp globalvars_off.h ; \
		cat ../inetport/02_struct.patch | grep -E "^S;" | ../inetport/dopatch.pl globalvars.h > /dev/null ; \
		cat ../inetport/02_struct.patch | grep -E "^S;" | ../inetport/dopatch.pl globalvars.c > /dev/null ; \
		../inetport/dopatch.pl . < ../inetport/04_params.patch ; \
		../inetport/dopatch.pl . < ../inetport/05_locals.lst ; \
		../inetport/dopatch.pl . < ../inetport/06_incl.patch ; \
		cp ../inetport/QUAGGA_VERSION . \
	)

clean:
	rm -Rf *.patch *.patch.in _source.xml xml
