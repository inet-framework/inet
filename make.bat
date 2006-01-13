@echo off
rem ***** ADJUST THE DIRECTORIES BELOW ACCORDING TO YOUR INSTALLATION ******
call "D:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" 
call "E:\Microsoft Platform SDK\SetEnv.Cmd" 

cd %~dp0
if not exist Makefile.vc call makemake.cmd
pause
nmake -f Makefile.vc %*
