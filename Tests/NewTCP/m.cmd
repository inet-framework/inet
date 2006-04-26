@echo off
call ..\..\..\omnetpp\setenv-vc71.bat

set root=..\..
call opp_nmakemake -f -N -w -u cmdenv -c %root%\inetconfig.vc -I..\.. -I%root%\Transport\TCP -I%root%\Transport\TCP\flavours -I%root%\Transport\TCP\queues -I%root%\Transport\Contract -I%root%\Network\Contract -I%root%\Network\IPv4 -I%root%\Network\IPv6 -I%root%\Base -I%root%\Util
nmake -f makefile.vc
