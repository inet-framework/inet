@echo off

rem
rem Args: <examplepath> <inifile> <configname>
rem

echo [General] >tmp.ini
echo cpu-time-limit = 3s >>tmp.ini

set INETROOT=%~dp0\..
cd %INETROOT%\%1
echo.
echo ========================================================
echo Running: %1 %2 %3
cd

%INETROOT%\src\inet -u Cmdenv -n %INETROOT%/src;%INETROOT%/examples -c %3 %2 %~dp0\tmp.ini

