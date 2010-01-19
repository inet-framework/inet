@echo off
PATH=C:\home\tools\libxslt-1.0.27.win32\util;%PATH%
del /q html\*
xsltproc -o netconf.html xsd2html.xsl netconf.xsd
for %%i in (html\*.dot) do call dot2gif %%i
perl proc.pl html
