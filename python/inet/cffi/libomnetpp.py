import cppyy

from inet.common.util import *

libsuffix = ""

# setup omnetpp include path
cppyy.add_include_path(get_omnetpp_relative_path("include"))
cppyy.add_include_path(get_omnetpp_relative_path("src"))

# setup omnetpp library path
cppyy.add_library_path(get_omnetpp_relative_path("lib"))

# load omnetpp libraries
cppyy.load_library("liboppcmdenv" + libsuffix)
cppyy.load_library("liboppcommon" + libsuffix)
cppyy.load_library("liboppenvir" + libsuffix)
cppyy.load_library("liboppeventlog" + libsuffix)
cppyy.load_library("libopplayout" + libsuffix)
cppyy.load_library("liboppnedxml" + libsuffix)
#cppyy.load_library("liboppqtenv-osg" + libsuffix)
#cppyy.load_library("liboppqtenv" + libsuffix)
#cppyy.load_library("liboppscave" + libsuffix)
cppyy.load_library("liboppsim" + libsuffix)

# include omnetpp header files
cppyy.include("omnetpp.h")
cppyy.include("cmdenv/cmdenv.h")
cppyy.include("envir/inifilereader.h")
cppyy.include("envir/sectionbasedconfig.h")

# setup omnetpp namespace
lib = cppyy.gbl.omnetpp
cStaticFlag = lib.cStaticFlag
SimTime = lib.SimTime
CodeFragments = lib.CodeFragments
cSimulation = lib.cSimulation
cEvent = lib.cEvent
Cmdenv = lib.cmdenv.Cmdenv
InifileReader = lib.envir.InifileReader
SectionBasedConfiguration = lib.envir.SectionBasedConfiguration
