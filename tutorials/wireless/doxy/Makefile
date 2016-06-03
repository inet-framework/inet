#
# Definitions
#
DOXYGEN=doxygen
OUTDIR=../html

#
# Targets
#
all:
	rm -rf $(OUTDIR)
	mkdir $(OUTDIR)
	$(DOXYGEN)
	cp *.gif *.png $(OUTDIR)
clean:
	-rm -rf $(OUTDIR)
