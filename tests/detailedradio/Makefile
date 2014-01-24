#
# OMNeT++/OMNEST Makefile for detailedradiotests
#
# This file was generated with the command:
#  opp_makemake -f --deep -o detailedradiotests -O out -I../../src/base -I../../src/util -I../../src/mobility/contract -I../../src/linklayer/contract -I../../src/linklayer/common -I../../src/physicallayer/contract -I../../src/physicallayer/common -I../../src/physicallayer/radio/contract -I../../src/physicallayer/radio/common -I../../src/physicallayer/radio/detailed -I../../src/physicallayer/radio/detailed/base -I../../src/physicallayer/radio/detailed/attenuation -I../../src/physicallayer/radio/detailed/radiochannel -I../../src/physicallayer/radio/detailed/mapping -I../../src/physicallayer/radio/detailed/decider -L../../out/$(CONFIGNAME)/src -linet
#

# Name of target to be created (-o option)
TARGET = detailedradiotests$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../../src/base \
    -I../../src/util \
    -I../../src/mobility/contract \
    -I../../src/linklayer/contract \
    -I../../src/linklayer/common \
    -I../../src/physicallayer/contract \
    -I../../src/physicallayer/common \
    -I../../src/physicallayer/radio/contract \
    -I../../src/physicallayer/radio/common \
    -I../../src/physicallayer/radio/detailed \
    -I../../src/physicallayer/radio/detailed/base \
    -I../../src/physicallayer/radio/detailed/attenuation \
    -I../../src/physicallayer/radio/detailed/radiochannel \
    -I../../src/physicallayer/radio/detailed/mapping \
    -I../../src/physicallayer/radio/detailed/decider \
    -I. \
    -IbasePhyLayer \
    -IchannelInfo \
    -IconnectionManager \
    -Icoord \
    -Idecider \
    -Imapping \
    -IradioState \
    -ItestUtils

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../out/$(CONFIGNAME)/src  -linet
LIBS += -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/src`

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = /tests/detailedradio
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/basePhyLayer/BasePhyTests.o \
    $O/basePhyLayer/TestMacLayer.o \
    $O/basePhyLayer/TestPhyLayer.o \
    $O/channelInfo/ChannelInfoTest.o \
    $O/connectionManager/CMPhyLayer.o \
    $O/connectionManager/ConnectedBCNodePhyLayer.o \
    $O/connectionManager/ConnectedRNodePhyLayer.o \
    $O/connectionManager/NotConnectedBCNodePhyLayer.o \
    $O/connectionManager/NotConnectedRNodePhyLayer.o \
    $O/connectionManager/TestCM.o \
    $O/coord/CoordTest.o \
    $O/decider/DeciderTest.o \
    $O/mapping/mappingtest.o \
    $O/radioState/TestPhyUtils.o \
    $O/testUtils/asserts.o \
    $O/testUtils/OmnetTestBase.o \
    $O/testUtils/TestManager.o \
    $O/testUtils/TestModule.o

# Message files
MSGFILES =

# SM files
SMFILES =

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f detailedradiotests detailedradiotests.exe libdetailedradiotests.so libdetailedradiotests.a libdetailedradiotests.dll libdetailedradiotests.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h ./*_sm.cc ./*_sm.h
	$(Q)-rm -f basePhyLayer/*_m.cc basePhyLayer/*_m.h basePhyLayer/*_sm.cc basePhyLayer/*_sm.h
	$(Q)-rm -f channelInfo/*_m.cc channelInfo/*_m.h channelInfo/*_sm.cc channelInfo/*_sm.h
	$(Q)-rm -f connectionManager/*_m.cc connectionManager/*_m.h connectionManager/*_sm.cc connectionManager/*_sm.h
	$(Q)-rm -f coord/*_m.cc coord/*_m.h coord/*_sm.cc coord/*_sm.h
	$(Q)-rm -f decider/*_m.cc decider/*_m.h decider/*_sm.cc decider/*_sm.h
	$(Q)-rm -f mapping/*_m.cc mapping/*_m.h mapping/*_sm.cc mapping/*_sm.h
	$(Q)-rm -f radioState/*_m.cc radioState/*_m.h radioState/*_sm.cc radioState/*_sm.h
	$(Q)-rm -f testUtils/*_m.cc testUtils/*_m.h testUtils/*_sm.cc testUtils/*_sm.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES) $(SM_CC_FILES)  ./*.cc basePhyLayer/*.cc channelInfo/*.cc connectionManager/*.cc coord/*.cc decider/*.cc mapping/*.cc radioState/*.cc testUtils/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/basePhyLayer/BasePhyTests.o: basePhyLayer/BasePhyTests.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  basePhyLayer/TestGlobals.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioSignal.h \
  ../../src/physicallayer/radio/contract/IRadioFrame.h \
  ../../src/util/FWMath.h \
  testUtils/asserts.h \
  ../../src/physicallayer/radio/detailed/ChannelSenseRequest_m.h \
  ../../src/physicallayer/radio/detailed/base/BasePhyLayer.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingBase.h \
  ../../src/linklayer/common/MACAddress.h \
  testUtils/OmnetTestBase.h \
  ../../src/physicallayer/radio/detailed/decider/Decider.h \
  ../../src/base/OperationalBase.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingUtils.h \
  ../../src/base/LifecycleOperation.h \
  ../../src/physicallayer/contract/IPhysicalLayerFrame.h \
  ../../src/physicallayer/radio/detailed/mapping/Interpolation.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/detailed/decider/DeciderToPhyInterface.h \
  testUtils/TestModule.h \
  basePhyLayer/TestDecider.h \
  basePhyLayer/../testUtils/TestManager.h \
  testUtils/TestManager.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelState.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  ../../src/base/ModuleAccess.h \
  basePhyLayer/BasePhyTests.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame_m.h \
  basePhyLayer/TestMacLayer.h \
  ../../src/physicallayer/radio/contract/IRadioSignal.h \
  basePhyLayer/../testUtils/asserts.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelInfo.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  ../../src/physicallayer/radio/detailed/PhyUtils.h \
  ../../src/base/Compat.h \
  ../../src/physicallayer/radio/detailed/attenuation/AnalogueModel.h \
  ../../src/base/InitStages.h \
  ../../src/base/NodeOperations.h \
  ../../src/physicallayer/radio/detailed/mapping/Mapping.h \
  ../../src/linklayer/common/MACFrameBase_m.h \
  basePhyLayer/TestPhyLayer.h \
  ../../src/base/ILifecycle.h \
  basePhyLayer/../testUtils/OmnetTestBase.h
$O/basePhyLayer/TestMacLayer.o: basePhyLayer/TestMacLayer.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  basePhyLayer/TestGlobals.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioSignal.h \
  ../../src/physicallayer/radio/contract/IRadioFrame.h \
  ../../src/util/FWMath.h \
  testUtils/asserts.h \
  ../../src/physicallayer/radio/detailed/ChannelSenseRequest_m.h \
  ../../src/physicallayer/radio/detailed/base/BasePhyLayer.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingBase.h \
  ../../src/linklayer/common/MACAddress.h \
  testUtils/OmnetTestBase.h \
  ../../src/physicallayer/radio/detailed/decider/Decider.h \
  ../../src/base/OperationalBase.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingUtils.h \
  ../../src/physicallayer/radio/common/PhyControlInfo_m.h \
  ../../src/base/LifecycleOperation.h \
  ../../src/physicallayer/contract/IPhysicalLayerFrame.h \
  ../../src/physicallayer/radio/detailed/mapping/Interpolation.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/util/FindModule.h \
  ../../src/physicallayer/radio/detailed/decider/DeciderToPhyInterface.h \
  testUtils/TestModule.h \
  basePhyLayer/TestDecider.h \
  testUtils/TestManager.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelState.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  ../../src/base/ModuleAccess.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame_m.h \
  basePhyLayer/TestMacLayer.h \
  ../../src/physicallayer/radio/contract/IRadioSignal.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelInfo.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  ../../src/base/Compat.h \
  ../../src/physicallayer/radio/detailed/PhyUtils.h \
  ../../src/physicallayer/radio/detailed/attenuation/AnalogueModel.h \
  ../../src/base/InitStages.h \
  ../../src/base/NodeOperations.h \
  ../../src/physicallayer/radio/detailed/mapping/Mapping.h \
  ../../src/linklayer/common/MACFrameBase_m.h \
  basePhyLayer/TestPhyLayer.h \
  ../../src/base/ILifecycle.h
$O/basePhyLayer/TestPhyLayer.o: basePhyLayer/TestPhyLayer.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioSignal.h \
  basePhyLayer/TestGlobals.h \
  ../../src/physicallayer/radio/contract/IRadioFrame.h \
  ../../src/util/FWMath.h \
  testUtils/asserts.h \
  ../../src/physicallayer/radio/detailed/ChannelSenseRequest_m.h \
  ../../src/physicallayer/radio/detailed/base/BasePhyLayer.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingBase.h \
  testUtils/OmnetTestBase.h \
  ../../src/physicallayer/radio/detailed/decider/Decider.h \
  ../../src/base/OperationalBase.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingUtils.h \
  ../../src/base/LifecycleOperation.h \
  ../../src/physicallayer/contract/IPhysicalLayerFrame.h \
  ../../src/physicallayer/radio/detailed/mapping/Interpolation.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/detailed/decider/DeciderToPhyInterface.h \
  testUtils/TestModule.h \
  basePhyLayer/TestDecider.h \
  testUtils/TestManager.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelState.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  ../../src/base/ModuleAccess.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame_m.h \
  ../../src/physicallayer/radio/contract/IRadioSignal.h \
  basePhyLayer/../testUtils/asserts.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelInfo.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  ../../src/base/Compat.h \
  ../../src/physicallayer/radio/detailed/PhyUtils.h \
  ../../src/physicallayer/radio/detailed/attenuation/AnalogueModel.h \
  ../../src/base/InitStages.h \
  ../../src/physicallayer/radio/detailed/mapping/Mapping.h \
  ../../src/base/NodeOperations.h \
  ../../src/base/ILifecycle.h \
  basePhyLayer/TestPhyLayer.h
$O/channelInfo/ChannelInfoTest.o: channelInfo/ChannelInfoTest.cc \
  ../../src/physicallayer/radio/detailed/DetailedRadioSignal.h \
  ../../src/physicallayer/radio/contract/IRadioFrame.h \
  ../../src/util/FWMath.h \
  testUtils/asserts.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingBase.h \
  testUtils/OmnetTestBase.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingUtils.h \
  ../../src/physicallayer/radio/detailed/mapping/Interpolation.h \
  ../../src/physicallayer/contract/IPhysicalLayerFrame.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame_m.h \
  ../../src/physicallayer/radio/contract/IRadioSignal.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelInfo.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h \
  ../../src/physicallayer/radio/detailed/mapping/Mapping.h
$O/connectionManager/CMPhyLayer.o: connectionManager/CMPhyLayer.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  ../../src/util/FWMath.h \
  ../../src/linklayer/common/MACAddress.h \
  ../../src/base/OperationalBase.h \
  ../../src/base/LifecycleOperation.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  ../../src/base/ModuleAccess.h \
  connectionManager/CMPhyLayer.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h \
  ../../src/base/NodeOperations.h \
  ../../src/linklayer/common/MACFrameBase_m.h \
  ../../src/base/ILifecycle.h
$O/connectionManager/ConnectedBCNodePhyLayer.o: connectionManager/ConnectedBCNodePhyLayer.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  ../../src/util/FWMath.h \
  connectionManager/ConnectedBCNodePhyLayer.h \
  ../../src/linklayer/common/MACAddress.h \
  ../../src/base/OperationalBase.h \
  ../../src/base/LifecycleOperation.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  connectionManager/CMPhyLayer.h \
  ../../src/base/ModuleAccess.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h \
  ../../src/base/NodeOperations.h \
  ../../src/linklayer/common/MACFrameBase_m.h \
  ../../src/base/ILifecycle.h
$O/connectionManager/ConnectedRNodePhyLayer.o: connectionManager/ConnectedRNodePhyLayer.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  ../../src/util/FWMath.h \
  ../../src/linklayer/common/MACAddress.h \
  ../../src/base/OperationalBase.h \
  ../../src/base/LifecycleOperation.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  connectionManager/ConnectedRNodePhyLayer.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  connectionManager/CMPhyLayer.h \
  ../../src/base/ModuleAccess.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h \
  ../../src/base/NodeOperations.h \
  ../../src/linklayer/common/MACFrameBase_m.h \
  ../../src/base/ILifecycle.h
$O/connectionManager/NotConnectedBCNodePhyLayer.o: connectionManager/NotConnectedBCNodePhyLayer.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  ../../src/util/FWMath.h \
  ../../src/linklayer/common/MACAddress.h \
  ../../src/base/OperationalBase.h \
  ../../src/base/LifecycleOperation.h \
  connectionManager/NotConnectedBCNodePhyLayer.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  connectionManager/CMPhyLayer.h \
  ../../src/base/ModuleAccess.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h \
  ../../src/base/NodeOperations.h \
  ../../src/linklayer/common/MACFrameBase_m.h \
  ../../src/base/ILifecycle.h
$O/connectionManager/NotConnectedRNodePhyLayer.o: connectionManager/NotConnectedRNodePhyLayer.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  ../../src/util/FWMath.h \
  ../../src/linklayer/common/MACAddress.h \
  ../../src/base/OperationalBase.h \
  ../../src/base/LifecycleOperation.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  connectionManager/CMPhyLayer.h \
  ../../src/base/ModuleAccess.h \
  connectionManager/NotConnectedRNodePhyLayer.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h \
  ../../src/base/NodeOperations.h \
  ../../src/linklayer/common/MACFrameBase_m.h \
  ../../src/base/ILifecycle.h
$O/connectionManager/TestCM.o: connectionManager/TestCM.cc \
  ../../src/physicallayer/radio/contract/IRadioFrame.h \
  ../../src/util/FWMath.h \
  testUtils/asserts.h \
  ../../src/physicallayer/radio/contract/IRadioChannel.h \
  ../../src/physicallayer/contract/IPhysicalLayerFrame.h \
  ../../src/mobility/contract/IMobility.h \
  connectionManager/TestCM.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  ../../src/physicallayer/radio/detailed/radiochannel/BaseConnectionManager.h \
  ../../src/physicallayer/radio/contract/IRadioSignal.h \
  ../../src/base/INETDefs.h \
  ../../src/base/Compat.h \
  ../../src/physicallayer/radio/detailed/radiochannel/NicEntry.h \
  ../../src/base/InitStages.h \
  ../../src/physicallayer/radio/common/RadioChannelBase.h
$O/coord/CoordTest.o: coord/CoordTest.cc \
  ../../src/util/FWMath.h \
  testUtils/asserts.h \
  testUtils/OmnetTestBase.h \
  ../../src/base/Coord.h \
  ../../src/base/INETDefs.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h
$O/decider/DeciderTest.o: decider/DeciderTest.cc \
  ../../src/physicallayer/common/PhysicalLayerBase.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioSignal.h \
  ../../src/physicallayer/radio/contract/IRadioFrame.h \
  ../../src/util/FWMath.h \
  ../../src/physicallayer/radio/detailed/ChannelSenseRequest_m.h \
  testUtils/asserts.h \
  ../../src/physicallayer/radio/detailed/base/BasePhyLayer.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingBase.h \
  decider/DeciderTest.h \
  ../../src/physicallayer/radio/detailed/decider/SNRThresholdDecider.h \
  ../../src/physicallayer/radio/detailed/decider/BaseDecider.h \
  testUtils/OmnetTestBase.h \
  ../../src/physicallayer/radio/detailed/decider/Decider.h \
  ../../src/base/OperationalBase.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingUtils.h \
  ../../src/base/LifecycleOperation.h \
  ../../src/physicallayer/contract/IPhysicalLayerFrame.h \
  ../../src/physicallayer/radio/detailed/mapping/Interpolation.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame.h \
  ../../src/physicallayer/radio/detailed/radiochannel/DetailedRadioChannelAccess.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/detailed/decider/DeciderToPhyInterface.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelState.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  ../../src/base/ModuleAccess.h \
  ../../src/physicallayer/radio/detailed/DetailedRadioFrame_m.h \
  ../../src/physicallayer/radio/contract/IRadioSignal.h \
  decider/TestSNRThresholdDeciderNew.h \
  ../../src/base/INETDefs.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelInfo.h \
  ../../src/physicallayer/radio/common/RadioBase.h \
  decider/../testUtils/asserts.h \
  ../../src/physicallayer/radio/detailed/PhyUtils.h \
  ../../src/base/Compat.h \
  ../../src/physicallayer/radio/detailed/attenuation/AnalogueModel.h \
  ../../src/base/InitStages.h \
  ../../src/base/NodeOperations.h \
  ../../src/physicallayer/radio/detailed/mapping/Mapping.h \
  ../../src/base/ILifecycle.h
$O/mapping/mappingtest.o: mapping/mappingtest.cc \
  ../../src/util/FWMath.h \
  ../../src/physicallayer/radio/detailed/ChannelSenseRequest_m.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingBase.h \
  ../../src/physicallayer/radio/detailed/decider/BaseDecider.h \
  ../../src/physicallayer/radio/detailed/decider/Decider.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingUtils.h \
  ../../src/physicallayer/radio/detailed/mapping/Interpolation.h \
  ../../src/physicallayer/radio/detailed/decider/Decider802154Narrow.h \
  ../../src/physicallayer/radio/detailed/decider/DeciderToPhyInterface.h \
  mapping/../testUtils/OmnetTestBase.h \
  ../../src/physicallayer/radio/detailed/radiochannel/ChannelState.h \
  mapping/../testUtils/asserts.h \
  ../../src/base/INETDefs.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h \
  ../../src/physicallayer/radio/detailed/mapping/Mapping.h
$O/radioState/TestPhyUtils.o: radioState/TestPhyUtils.cc \
  ../../src/util/FWMath.h \
  testUtils/asserts.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingBase.h \
  testUtils/OmnetTestBase.h \
  ../../src/physicallayer/radio/detailed/mapping/MappingUtils.h \
  ../../src/physicallayer/radio/detailed/mapping/Interpolation.h \
  ../../src/mobility/contract/IMobility.h \
  ../../src/physicallayer/radio/contract/IRadio.h \
  ../../src/base/Coord.h \
  ../../src/physicallayer/contract/IPhysicalLayer.h \
  ../../src/base/INETDefs.h \
  ../../src/base/Compat.h \
  ../../src/physicallayer/radio/detailed/PhyUtils.h \
  ../../src/base/InitStages.h \
  ../../src/physicallayer/radio/detailed/attenuation/AnalogueModel.h \
  ../../src/physicallayer/radio/detailed/mapping/Mapping.h
$O/testUtils/OmnetTestBase.o: testUtils/OmnetTestBase.cc \
  testUtils/asserts.h \
  testUtils/OmnetTestBase.h \
  ../../src/base/INETDefs.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h
$O/testUtils/TestManager.o: testUtils/TestManager.cc \
  testUtils/asserts.h \
  testUtils/OmnetTestBase.h \
  testUtils/TestManager.h \
  ../../src/base/INETDefs.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h
$O/testUtils/TestModule.o: testUtils/TestModule.cc \
  testUtils/asserts.h \
  testUtils/OmnetTestBase.h \
  ../../src/util/FindModule.h \
  testUtils/TestModule.h \
  testUtils/TestManager.h \
  ../../src/base/INETDefs.h \
  ../../src/base/Compat.h \
  ../../src/base/InitStages.h
$O/testUtils/asserts.o: testUtils/asserts.cc \
  testUtils/asserts.h
