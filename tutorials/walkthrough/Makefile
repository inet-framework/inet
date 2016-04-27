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
clean:
	-rm -rf out
