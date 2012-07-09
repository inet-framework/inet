# examples for the xlib++ library

all : example1 example2 example3 example4 example5 example6

example1_objects = example1.o
example2_objects = example2.o
example3_objects = example3.o
example4_objects = example4.o
example5_objects = example5.o
example6_objects = example6.o



#CXXFLAGS= -g -I/usr/X11R6/include -I/usr/local/include -fsjlj-exceptions
CXXFLAGS= -g -I/usr/X11R6/include -I/home/archmage/cit/trunk/papers/ntc2010-delfoi/src



example1: $(example1_objects)
	g++ -L/usr/local/lib -L/usr/X11R6/lib \
	-o example1 $(example1_objects) -lX11

example2: $(example2_objects)
	g++ -L/usr/local/lib -L/usr/X11R6/lib \
	-o example2 $(example2_objects) -lX11

example3: $(example3_objects)
	g++ -L/usr/local/lib -L/usr/X11R6/lib \
	-o example3 $(example3_objects) -lX11

example4: $(example4_objects)
	g++ -L/usr/local/lib -L/usr/X11R6/lib \
	-o example4 $(example4_objects) -lX11

example5: $(example5_objects)
	g++ -L/usr/local/lib -L/usr/X11R6/lib \
	-o example5 $(example5_objects) -lX11

example6: $(example6_objects)
	g++ -L/usr/local/lib -L/usr/X11R6/lib \
	-o example6 $(example6_objects) -lX11


example1.o : example1.cpp
example2.o : example2.cpp
example3.o : example3.cpp
example4.o : example4.cpp
example5.o : example5.cpp
example6.o : example6.cpp


clean :
	rm -f *.o example1 example2 example3 example4 example5 example6
