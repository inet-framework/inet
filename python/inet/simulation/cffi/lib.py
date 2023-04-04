import cppyy

from inet.common.util import *

# setup inet include path
cppyy.add_include_path(get_workspace_path("inet/src"))

# setup inet library path
cppyy.add_library_path(get_workspace_path("inet/src"))

# load inet library
cppyy.load_library("libINET")

# setup inet namespace
# from cppyy.gbl import inet
