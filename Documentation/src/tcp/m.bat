@echo off
call C:\home\omnetpp\setenv-vc71.bat
doxygen doxyfile
copy *.png ..\..\tcp-tutorial
start ..\..\tcp-tutorial\index.html
