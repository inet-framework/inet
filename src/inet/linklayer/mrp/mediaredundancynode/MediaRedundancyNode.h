// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_MRPNODE_H
#define __INET_MRPNODE_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ethernet/common/MacForwardingTable.h"
#include "inet/linklayer/mrp/common/MrpMacForwardingTable.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/linklayer/configurator/MrpInterfaceData.h"
#include "inet/linklayer/mrp/common/MrpRelay.h"
#include "inet/linklayer/mrp/common/MrpPdu_m.h"
#include "inet/linklayer/mrp/common/ContinuityCheckMessage_m.h"

namespace inet {

/**
 * Base class for MRC and MRM.
 */

class INET_API MediaRedundancyNode : public OperationalBase, public cListener
{
protected:
    enum frameType : uint64_t {
        MC_RESERVED = 0x000001154E000000,
        MC_TEST = 0x000001154E000001,
        MC_CONTROL = 0x000001154E000002,
        MC_INTEST = 0x000001154E000003,
        MC_INCONTROL = 0x000001154E000004,
        MC_INTRANSFER = 0x000001154E000005,
    };

    struct Uuid{
        uint64_t uuid0 = 0;
        uint64_t uuid1 = 0;
    };

    enum ringState:uint16_t{
        OPEN = 0x0000,
        CLOSED = 0x0001,
        UNDEFINED = 0x0003,
    };

    enum mrpRole: uint16_t{
        DISABLED = 0,
        CLIENT = 1,
        MANAGER = 2,
        MANAGER_AUTO_COMP = 3,
        MANAGER_AUTO = 4,
    };

    enum mrpPriority : uint16_t {
        HIGHEST = 0x0000,
        HIGH = 0x4000,
        DEFAULT = 0x8000,
    };

    enum nodeState{
        POWER_ON,
        AC_STAT1, //waiting for the first Link Up at one of its ring ports, starting test monitoring of the ring

        //Manager States
        PRM_UP, //only the primary ring port has a link. The MRM shall send MRP_Test frames periodically through both ring ports
        CHK_RO, //Check Ring, Ring Open State
        CHK_RC, //Check Ring, Ring Closed State

        //Client States
        DE_IDLE, //Data Exchange Idle: This state shall be reached if only one ring port (primary) has a link and its port state is set to FORWARDING
        PT, //Pass Through: Temporary state while signaling link changes
        DE, //Data Exchange: Temporary state while signaling link changes
        PT_IDLE, //Pass Through Idle: both ring ports have a link and their port states are set to FORWARDING
    };

    const static uint16_t VLAN_LT = 0x8100;
    const static uint8_t TAG_CONTROL_PRIO = 0x07;
    const static uint16_t MRP_LT = 0x88E3;

    bool visualize = false;
    Uuid domainID;
    std::string domainName[241];
    int timingProfile;
    uint16_t vlanID = 0;

    mrpRole expectedRole = DISABLED;
    nodeState currentState = POWER_ON;
    bool interconnectionLinkCheckAware = true;
    bool interConnectionRingCheckAware = true;
    bool enableLinkCheckOnRing = false;
    MacAddress sourceAddress;

    opp_component_ptr<NetworkInterface> ringInterface1 = nullptr;
    opp_component_ptr<NetworkInterface> ringInterface2 = nullptr;
    int primaryRingPort;
    int secondaryRingPort;
    uint8_t priority = 7;

    double linkDownInterval = 20; //MRP_LNKdownT
    double linkUpInterval = 20; //MRP_LNKupT
    double topologyChangeInterval; //MRP_TOPchgT
    double shortTestInterval; //MRP_TSTshortT
    double defaultTestInterval; //MRP_TSTdefaultT
    double ccmInterval = 10; // time in milliseconds. either 3.3 or 10 ms

    uint16_t topologyChangeMaxRepeatCount = 3; //MRP_TOPNRmax
    uint16_t topologyChangeRepeatCount = 0; //MRP_TOPNReturn

    uint16_t testMaxRetransmissionCount =0; //MRP_MRM_NRmax
    uint16_t testRetransmissionCount = 0; //MRP_MRM_NReturn

    uint16_t linkMaxChange = 4; //MRP_LNKNRmax
    uint16_t linkChangeCount; //MRP_LNKNReturn

    uint16_t testMonitoringCount; //MRP_TSTNRmax
    uint16_t testMonitoringExtendedCount = 15; //MRP_TSTExtNRmax

    cMessage *linkDownTimer = nullptr;
    cMessage *linkUpTimer = nullptr;
    cMessage *fdbClearTimer= nullptr;
    cMessage *fdbClearDelay = nullptr;
    cMessage *topologyChangeTimer = nullptr;
    cMessage *testTimer = nullptr;
    cMessage *startUpTimer = nullptr;

    ringState currentRingState = OPEN;
    uint16_t transition = 0;
    uint16_t sequenceID=0;
    uint16_t ccm1ID=0;
    uint16_t ccm2ID=0;

    opp_component_ptr<cModule> switchModule;
    ModuleRefByPar<MrpMacForwardingTable> mrpMacForwardingTable;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<MrpRelay> relay;

    //Variables need for Manager
    bool addTest = false;
    bool nonBlockingMRC = true;
    bool reactOnLinkChange = true;
    bool checkMediaRedundancy=false;
    bool noTopologyChange = false;
    mrpPriority managerPrio = DEFAULT;

    //Variables needed for Client
    bool blockedStateSupported;

    //Variables needed for Automanager
    MacAddress hostBestMRMSourceAddress;
    mrpPriority hostBestMRMPriority;
    uint16_t monNReturn;
    uint16_t monNRmax = 5;
    cMessage *linkUpHysterisisTimer = nullptr; //LNKUP_HYST_TIMER_RUNNING

    //CCM
    int maintenanceDomain;
    int maintenanceAssociation;
    int sequenceCCM1 =0;
    int sequenceCCM2 =0;
    const MacAddress ccmMulticastAddress = MacAddress("01:80:C2:00:00:30");

    double linkDetectionDelay = 0;
    double processingDelay = 0;
    double processingDelayMean = 100;
    double processingDelayDev = 80;
    double linkDetectionDelayMean = 385000;
    double linkDetectionDelayDev = 325000;

    uint16_t lastTestId =0;
    uint16_t lastTopologyId =0;

    typedef std::map<uint16_t, int64_t> FrameSentDatabase;
    FrameSentDatabase testFrameSent;
    simsignal_t LinkChangeSignal;
    simsignal_t TopologyChangeSignal;
    simsignal_t TestSignal;
    simsignal_t ContinuityCheckSignal;
    simsignal_t ReceivedChangeSignal;
    simsignal_t ReceivedTestSignal;
    simsignal_t ReceivedContinuityCheckSignal;
    simsignal_t RingStateChangedSignal;
    simsignal_t PortStateChangedSignal;
    simsignal_t ClearFDBSignal;

public:
    typedef MrpInterfaceData::PortInfo PortInfo;

protected:
    virtual void start();
    virtual void stop();
    virtual void read();
    virtual void initialize(int stage) override;
    virtual void mrcInit();
    virtual void mrmInit();
    virtual void mraInit();
    virtual void initPortTable();
    virtual void initRingPorts();
    virtual void initContinuityCheck();
    virtual void initInterfacedata(unsigned int interfaceId);
    virtual void toggleRingPorts();
    virtual void setPortRole(int InterfaceId, MrpInterfaceData::PortRole Role);
    virtual void setPortState(int InterfaceId, MrpInterfaceData::PortState State);
    virtual MrpInterfaceData::PortState getPortState(int InterfaceId);
    virtual MrpInterfaceData::PortRole getPortRole(int InterfaceId);
    virtual NetworkInterface *getPortNetworkInterface(unsigned int interfaceId) const;
    virtual MrpInterfaceData *getPortInterfaceDataForUpdate(unsigned int interfaceId);
    virtual const MrpInterfaceData *getPortInterfaceData(unsigned int interfaceId) const;
    virtual void handleMrpPDU(Packet *packet);
    virtual void handleContinuityCheckMessage(Packet *packet);
    virtual void handleDelayTimer(int interfaceId, int field);
    virtual void handleTopologyChangeTimer();
    virtual void clearLocalFDB();
    virtual void clearLocalFDBDelayed();
    virtual void handleTestTimer();
    virtual void handleLinkUpTimer();
    virtual void handleLinkDownTimer();
    virtual void handleContinuityCheckTimer(int RingPort);
    virtual void setupTestRingReq();
    virtual void setupTopologyChangeReq(uint32_t Interval);
    virtual void setupContinuityCheck(int RingPort);
    virtual void testRingReq(double Time);
    virtual void testMgrNackReq(mrpPriority ManagerPrio, MacAddress SourceAddress);
    virtual void testPropagateReq(mrpPriority ManagerPrio, MacAddress SourceAddress);
    virtual void topologyChangeReq(double Time);
    virtual void linkChangeReq(int RingPort, uint16_t LinkState);
    virtual void setupLinkChangeReq(int RingPort, uint16_t LinkState, double Time);
    virtual void testRingInd(MacAddress SourceAddress, mrpPriority ManagerPrio);
    virtual void topologyChangeInd(MacAddress SourceAddress, double Time);
    virtual void linkChangeInd(uint16_t PortMode, uint16_t LinkState);
    virtual void testMgrNackInd(int RingPort, MacAddress SourceAddress, mrpPriority ManagerPrio, MacAddress BestMRMSourceAddress);
    virtual void testPropagateInd(int RingPort, MacAddress SourceAddress, mrpPriority ManagerPrio, MacAddress BestMRMSourceAddress, mrpPriority BestMRMPrio);
    virtual void mauTypeChangeInd(int RingPort, uint16_t LinkState);
    virtual void interconnTopologyChangeInd(MacAddress SourceAddress, double Time, uint16_t InID, int RingPort, Packet* packet);
    virtual void interconnLinkChangeInd(uint16_t InID, uint16_t Linkstate,int RingPort, Packet* packet);
    virtual void interconnLinkStatusPollInd(uint16_t InID, int RingPort, Packet* packet);
    virtual void interconnTestInd(MacAddress SourceAddress, int RingPort, uint16_t InID, Packet* packet);
    virtual void interconnForwardReq(int RingPort, Packet* Packet);
    virtual void sendFrameReq(int portId, const MacAddress& DestinationAddress, const MacAddress& SourceAddress,int Prio, uint16_t LT, Packet *MRPPDU);
    virtual void sendCCM(int PortId, Packet *CCM);
    virtual void clearFDB(double Time);
    virtual void colorLink(NetworkInterface *ie, bool forwarding) const;
    virtual void refreshDisplay() const override;

public:
    MediaRedundancyNode();
    virtual ~MediaRedundancyNode();
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void setTimingProfile(int maxRecoveryTime);
    virtual void setRingInterfaces(int InterfaceIndex1, int InterfaceIndex2);
    virtual void setRingInterface(int InterfaceNumber, int InterfaceIndex);
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LAST; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
};

} // namespace inet

#endif

