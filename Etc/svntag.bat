@echo off
if "%1"=="" echo usage: %~n0 tagname && exit
set BASE=https://dev.omnetpp.org/svn/inet-framework
echo.
echo Tagging current directory with "%1":
echo ---^> svn copy %BASE%/trunk %BASE%/tags/%1
echo.
pause
echo on
svn copy %BASE%/trunk %BASE%/tags/%1 -m "tag:%1"
