#!/bin/sh

pause press ENTER if you really want this

find ../../src -name *.ned >__tmp__
find ../../src -name *.msg >>__tmp__
find ../../examples -name *.ned >>__tmp__
find ../../examples -name *.msg >>__tmp__

perl ./tweakfiles.pl __tmp__

rm __tmp__