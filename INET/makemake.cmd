@echo off
cd %~dp0
if exist C:\home\omnetpp40\omnetpp\setenv-vc71.bat call C:\home\omnetpp40\omnetpp\setenv-vc71.bat
nmake /? >nul 2>nul || echo *** ERROR: nmake.exe is not in the PATH *** && goto end
call opp_test >nul 2>nul || echo *** ERROR: OMNeT++/bin is not in the PATH *** && goto end

echo Generating makefile...
cd src && opp_nmakemake -f --deep || echo *** ERROR GENERATING MAKEFILES *** && goto end

echo.
echo ================================================================
echo Makefiles created -- type "nmake -f Makefile.vc" to compile.
echo ================================================================

:end