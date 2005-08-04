@echo off
call d:\home\omnetpp\setenv-vc71.bat
doxygen doxyfile
copy *.png ..\..\tcp-tutorial
start ..\..\tcp-tutorial\index.html
