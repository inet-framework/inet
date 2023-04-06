import cppyy

from omnetpp.runtime import *

from inet.common.util import *

libsuffix = "_dbg"

# setup inet include path
cppyy.add_include_path(get_inet_relative_path("src"))

# setup inet library path
cppyy.add_library_path(get_inet_relative_path("src"))

# load inet library
cppyy.load_library("libINET" + libsuffix)

CodeFragments.executeAll(CodeFragments.EARLY_STARTUP)

# setup inet namespace
# from cppyy.gbl import inet
