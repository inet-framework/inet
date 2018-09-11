s/\\begin\{verbatim\}/\\begin{verbatim}###verbatim###/g;
s/\\end\{verbatim\}/\\end{verbatim}/g;

s/\\begin\{ned\}/\\begin{verbatim}###ned###/g;
s/\\end\{ned\}/\\end{verbatim}/g;

s/\\begin\{cpp\}/\\begin{verbatim}###cpp###/g;
s/\\end\{cpp\}/\\end{verbatim}/g;

s/\\begin\{inifile\}/\\begin{verbatim}###ini###/g;
s/\\end\{inifile\}/\\end{verbatim}/g;

s/\\begin\{XML\}/\\begin{verbatim}###XML###/g;
s/\\end\{XML\}/\\end{verbatim}/g;

s/\\begin\{note\}/\\begin{verbatim}###note###/g;
s/\\end\{note\}/\\end{verbatim}/g;

s/\\begin\{important\}/\\begin{verbatim}###important###/g;
s/\\end\{important\}/\\end{verbatim}/g;

s/\\begin\{warning\}/\\begin{verbatim}###warning###/g;
s/\\end\{warning\}/\\end{verbatim}/g;

s/\\begin\{caution\}/\\begin{verbatim}###caution###/g;
s/\\end\{caution\}/\\end{verbatim}/g;

s/\\ttt\{(.+?)\}/:ttt:§$1§/g;
s/\\cppclass\{(.+?)\}/:cpp:§$1§/g;
s/\\ffunc\{(.+?)\}/:func:§$1§/g;
s/\\ffilename\{(.+?)\}/:filename:§$1§/g;
s/\\fvar\{(.+?)\}/:var:§$1§/g;
s/\\fpar\{(.+?)\}/:par:§$1§/g;
s/\\fgate\{(.+?)\}/:gate:§$1§/g;
s/\\protocol\{(.+?)\}/:protocol:§$1§/g;
s/\\nedtype\{(.+?)\}/:ned:§$1§/g;
s/\\msgtype\{(.+?)\}/:msg:§$1§/g;
