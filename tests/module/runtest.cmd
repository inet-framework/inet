@echo off
rem
rem usage: runtest [<testfile>...]
rem without args, runs all *.test files in the current directory
rem

set TESTFILES=%*
if "x%TESTFILES%" == "x" set TESTFILES=*.test
mkdir work 2>nul
xcopy /i /y lib work\lib
: del work\work.exe 2>nul

call opp_test -g -v %TESTFILES% || goto end

echo.
cd work || goto end
call opp_nmakemake -f --deep -linet -L../../../src -P . --no-deep-includes -I../../../src
nmake -f makefile.vc || cd .. && goto end
cd .. || goto end

echo.
path %~dp0\..\..\src;%PATH%
call opp_test -r %OPT% -v %TESTFILES% || goto end

echo.
echo Results can be found in work/

:end