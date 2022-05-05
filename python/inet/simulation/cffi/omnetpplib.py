import cppyy

from inet.common.util import *

# setup omnetpp include path
cppyy.add_include_path(get_workspace_path("omnetpp/include"))
cppyy.add_include_path(get_workspace_path("omnetpp/src"))

# setup omnetpp library path
cppyy.add_library_path(get_workspace_path("omnetpp/lib"))

# load omnetpp libraries
cppyy.load_library("liboppcmdenv")
cppyy.load_library("liboppcommon")
cppyy.load_library("liboppenvir")
cppyy.load_library("liboppeventlog")
cppyy.load_library("libopplayout")
cppyy.load_library("liboppnedxml")
#cppyy.load_library("liboppqtenv-osg")
#cppyy.load_library("liboppqtenv")
#cppyy.load_library("liboppscave")
cppyy.load_library("liboppsim")

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
