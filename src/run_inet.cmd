@echo off
if exist %~dp0\inet.exe (
    %~dp0\inet -n %~dp0\..\examples;%~dp0 %*
) else (
    opp_run -l %~dp0\inet -n %~dp0\..\examples;%~dp0 %*
)
