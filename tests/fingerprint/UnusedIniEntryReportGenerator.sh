#!/bin/bash
./fingerprinttest -d -f tplx   -a --print-unused-config=false --print-unused-config-on-completion=true
grep -r -e '^\[INFO\].*=.*\(from .*\)$' results/ | grep -v '(from <command-line>' | sort >unused_ini_lines.txt
