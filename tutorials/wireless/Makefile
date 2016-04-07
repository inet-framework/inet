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
	$(DOXYGEN) wirelesstutorial.doxyfile
	cp *.gif *.png out
clean:
	-rm -rf out
