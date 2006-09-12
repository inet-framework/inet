@echo off
if "%1"=="" echo usage: %~n0 tagname && exit
echo.
echo Tagging current directory with "%1":
echo ---^> svn copy . https://dev.omnetpp.org/svn/inet-framework/tags/%1
echo.
pause
svn copy . https://dev.omnetpp.org/svn/inet-framework/tags/%1 -m "tag:%1"
