@echo off
pushd images && for %%i in (*.gif) do echo %%i && "C:\Program Files\IrfanView\i_view32.exe" %%~ni.gif /resample=(60p,60p) /aspectratio /convert=..\thumbs\%%~ni.gif


