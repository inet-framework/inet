import cppyy

# setup omnetpp include path
cppyy.add_include_path("/home/levy/workspace/omnetpp/include")
cppyy.add_include_path("/home/levy/workspace/omnetpp/src")

# setup omnetpp library path
cppyy.add_library_path("/home/levy/workspace/omnetpp/lib")

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
from cppyy.gbl import omnetpp
cStaticFlag = omnetpp.cStaticFlag
SimTime = omnetpp.SimTime
CodeFragments = omnetpp.CodeFragments
cSimulation = omnetpp.cSimulation
cEvent = omnetpp.cEvent
Cmdenv = omnetpp.cmdenv.Cmdenv
InifileReader = omnetpp.envir.InifileReader
SectionBasedConfiguration = omnetpp.envir.SectionBasedConfiguration

# setup inet include path
cppyy.add_include_path("/home/levy/workspace/inet/src")

# setup omnetpp library path
cppyy.add_library_path("/home/levy/workspace/inet/src")

# load inet library
cppyy.load_library("libINET")

# setup inet namespace
# from cppyy.gbl import inet
