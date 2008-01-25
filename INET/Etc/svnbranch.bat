@echo off
if "%1"=="" echo usage: %~n0 branchname && exit
echo.
echo Tagging current directory with "%1":
echo ---^> svn copy . https://dev.omnetpp.org/svn/inet-framework/branches/%1
echo.
pause
svn copy . https://dev.omnetpp.org/svn/inet-framework/branches/%1 -m "branch:%1"
