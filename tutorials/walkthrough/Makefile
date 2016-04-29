#
# Definitions
#
DOXYGEN=doxygen

#
# Targets
#
all:
	rm -rf out
	mkdir out
	$(DOXYGEN)
	cp images/*.gif images/*.png out
	cp ../lib/lightbox2/* out
clean:
	-rm -rf out
