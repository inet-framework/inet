@echo off
if not "x%1" == "x" echo Please change to the directory you want to convert and run the script there without any command line argument! && goto endlabel
echo.
echo Press ENTER to convert all SCA files under the current directory:
cd
echo.
pause

dir /s /b *.sca >scafiles.lst

echo The following files will be checked/modified:
type scafiles.lst | more

echo.
echo Press ENTER to start the conversion, or CTRL-C to quit.
pause

perl %~dp0\migratesca.pl scafiles.lst

:endlabel
