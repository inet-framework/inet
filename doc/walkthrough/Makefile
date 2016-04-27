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
	$(DOXYGEN) walkthrough.doxyfile
	cp *.gif *.png out
clean:
	-rm -rf out
