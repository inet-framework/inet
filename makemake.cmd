cd %~dp0
call ..\omnetpp\setenv-vc71.bat
nmake ROOT=%~dp0 MAKEMAKE=opp_nmakemake EXT=.vc -f makemakefiles || echo *** ERROR GENERATING MAKEFILES ***

echo.
echo Generating list of framework NED files (nedfiles.lst)...
dir /s/b *.ned > nedfiles.lst
perl Etc\processNEDFileList.pl %~dp0\nedfiles.lst %~dp0

