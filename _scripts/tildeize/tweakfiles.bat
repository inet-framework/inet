@echo off
pause press ENTER if you really want this

dir /b /s src\*.ned >__tmp__
dir /b /s src\*.msg >>__tmp__
dir /b /s examples\*.ned >>__tmp__
dir /b /s examples\*.msg >>__tmp__

C:\mingw\msys\bin\perl.exe %~dp0\tweakfiles.pl __tmp__
del __tmp__