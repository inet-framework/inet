@echo off
cd %~dp0
if exist ..\omnetpp\setenv-vc71.bat call ..\omnetpp\setenv-vc71.bat
nmake /? >nul 2>nul || echo *** ERROR: nmake.exe is not in the PATH *** && goto end
call opp_test >nul 2>nul || echo *** ERROR: OMNeT++/bin is not in the PATH *** && goto end

if not exist inetconfig.vc perl Etc\create-inetconfigvc.pl || echo *** ERROR CREATING DEFAULT inetconfig.vc FROM inetconfig.vc-SAMPLE *** && goto end

echo Generating makefiles...
nmake /nologo ROOT=%~dp0 MAKEMAKE=opp_nmakemake EXT=.vc -f makemakefiles || echo *** ERROR GENERATING MAKEFILES *** && goto end

echo Adding dependencies to makefiles...
nmake /nologo -f Makefile.vc depend || echo *** ERROR ADDING DEPENDENCIES TO MAKEFILES *** && goto end

echo Generating list of framework NED files (nedfiles.lst)...
dir /s/b *.ned > nedfiles.lst
perl Etc\processNEDFileList.pl %~dp0\nedfiles.lst %~dp0 || echo *** ERROR CREATING NEDFILES.LST *** && goto end

echo.
echo ================================================================
echo Makefiles created -- type "nmake -f Makefile.vc" to compile.
echo ================================================================

:end