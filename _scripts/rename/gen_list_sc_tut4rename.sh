#!/bin/sh

find ../../showcases -name "*.ned" -o -name "*.ini" -o -name "*.xml" -o -name "*.md" >filelist_sc_tut.txt
find ../../tutorials -name "*.ned" -o -name "*.ini" -o -name "*.xml" -o -name "*.md" >>filelist_sc_tut.txt
