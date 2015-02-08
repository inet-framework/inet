@echo off
SETLOCAL
	set OMNETPP_ROOT=C:\omnetpp
	set INET_ROOT=C:\Tools\omnetpp\inet-hnrl
REM
REM Note that linked directory name does not work (i.g., "C:\Users\kks\inet-hnrl").
REM
	set INET_EX= "%INET_ROOT%\examples"
	set INET_SRC="%INET_ROOT%\src"
	set INET=opp_run -l %INET_SRC%\inet -n %INET_EX%;%INET_SRC% -u Cmdenv
	set PATH=%OMNETPP_ROOT%/bin;%OMNETPP_ROOT%/tools/win32/mingw32/bin;%OMNETPP_ROOT%/tools/win32/usr/local/bin;%INET_ROOT%/src;%PATH%

	%INET% %*
ENDLOCAL
