// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_MRP_H
#define __INET_MRP_H

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

class INET_API Mrp: public OperationalBase, public cListener {
protected:
    typedef std::map<uint16_t, int64_t> FrameSentDatabase;

    enum FrameType : uint64_t {
        MC_RESERVED = 0x000001154E000000,
        MC_TEST = 0x000001154E000001,
        MC_CONTROL = 0x000001154E000002,
        MC_INTEST = 0x000001154E000003,
        MC_INCONTROL = 0x000001154E000004,
        MC_INTRANSFER = 0x000001154E000005,
    };

    struct Uuid {
        uint64_t uuid0 = 0;
        uint64_t uuid1 = 0;
    };

    enum RingState : uint16_t {
        OPEN = 0x0000,
        CLOSED = 0x0001,
        UNDEFINED = 0x0003,
    };

    enum MrpRole : uint16_t {
        DISABLED = 0,
        CLIENT = 1,
        MANAGER = 2,
        MANAGER_AUTO_COMP = 3,
        MANAGER_AUTO = 4,
    };

    enum MrpPriority : uint16_t {
        HIGHEST = 0x0000,
        HIGH = 0x4000,
        DEFAULT = 0x8000,
        MRAHIGHEST = 0x9000,
        MRADEFAULT = 0xA000,
        MRALOWEST = 0xFFFF,
    };

    enum NodeState {
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
    uint8_t priority = 7;
    bool interconnectionLinkCheckAware = true;
    bool interconnectionRingCheckAware = true;
    bool enableLinkCheckOnRing = false;
    simtime_t linkDetectionDelay;
    simtime_t processingDelay;
    MrpRole expectedRole = DISABLED;

    NodeState currentState = POWER_ON;
    RingState currentRingState = OPEN;
    uint16_t transition = 0;
    uint16_t sequenceID = 0;
    uint16_t lastTopologyId = 0;
    uint16_t ccm1ID = 0;
    uint16_t ccm2ID = 0;
    MacAddress sourceAddress;
    opp_component_ptr<NetworkInterface> ringInterface1 = nullptr;
    opp_component_ptr<NetworkInterface> ringInterface2 = nullptr;
    int primaryRingPort;
    int secondaryRingPort;

    //CCM
    int maintenanceDomain;
    int maintenanceAssociation;
    int sequenceCCM1 = 0;
    int sequenceCCM2 = 0;
    const MacAddress ccmMulticastAddress = MacAddress("01:80:C2:00:00:30");

    //Variables need for Manager
    bool addTest = false;
    bool nonBlockingMRC = true;
    bool reactOnLinkChange = true;
    bool checkMediaRedundancy = false;
    bool noTopologyChange = false;
    MrpPriority managerPrio = DEFAULT;

    //Variables needed for Automanager
    MacAddress hostBestMRMSourceAddress;
    MrpPriority hostBestMRMPriority;
    uint16_t monNReturn;
    uint16_t monNRmax = 5;
    cMessage *linkUpHysteresisTimer = nullptr; //LNKUP_HYST_TIMER_RUNNING

    //Variables needed for Client
    bool blockedStateSupported;

    double linkDownInterval = 20; //MRP_LNKdownT
    double linkUpInterval = 20; //MRP_LNKupT
    double topologyChangeInterval; //MRP_TOPchgT
    double shortTestInterval; //MRP_TSTshortT
    double defaultTestInterval; //MRP_TSTdefaultT
    double ccmInterval = 10; // time in milliseconds. either 3.3 or 10 ms

    uint16_t topologyChangeMaxRepeatCount = 3; //MRP_TOPNRmax
    uint16_t topologyChangeRepeatCount = 0; //MRP_TOPNReturn
    uint16_t testMaxRetransmissionCount = 0; //MRP_MRM_NRmax
    uint16_t testRetransmissionCount = 0; //MRP_MRM_NReturn
    uint16_t linkMaxChange = 4; //MRP_LNKNRmax
    uint16_t linkChangeCount; //MRP_LNKNReturn
    uint16_t testMonitoringCount; //MRP_TSTNRmax
    uint16_t testMonitoringExtendedCount = 15; //MRP_TSTExtNRmax

    cMessage *linkDownTimer = nullptr;
    cMessage *linkUpTimer = nullptr;
    cMessage *fdbClearTimer = nullptr;
    cMessage *fdbClearDelay = nullptr;
    cMessage *topologyChangeTimer = nullptr;
    cMessage *testTimer = nullptr;
    cMessage *startUpTimer = nullptr;

    opp_component_ptr<cModule> switchModule;
    ModuleRefByPar<MrpMacForwardingTable> mrpMacForwardingTable;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<MrpRelay> relay;

    FrameSentDatabase testFrameSent;
    simsignal_t linkChangeSignal;
    simsignal_t topologyChangeSignal;
    simsignal_t testSignal;
    simsignal_t continuityCheckSignal;
    simsignal_t receivedChangeSignal;
    simsignal_t receivedTestSignal;
    simsignal_t receivedContinuityCheckSignal;
    simsignal_t ringStateChangedSignal;
    simsignal_t portStateChangedSignal;
    simsignal_t clearFDBSignal;

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
    virtual void startContinuityCheck();
    virtual void initInterfacedata(unsigned int interfaceId);
    virtual void toggleRingPorts();
    virtual void setPortRole(int interfaceId, MrpInterfaceData::PortRole role);
    virtual void setPortState(int interfaceId, MrpInterfaceData::PortState state);
    virtual MrpInterfaceData::PortState getPortState(int interfaceId);
    virtual MrpInterfaceData::PortRole getPortRole(int interfaceId);
    virtual NetworkInterface* getPortNetworkInterface(unsigned int interfaceId) const;
    virtual MrpInterfaceData* getPortInterfaceDataForUpdate(unsigned int interfaceId);
    virtual const MrpInterfaceData* getPortInterfaceData(unsigned int interfaceId) const;
    virtual void handleMrpPDU(Packet* packet);
    virtual void handleContinuityCheckMessage(Packet* packet);
    virtual void handleDelayTimer(int ringPort, int field);
    virtual void handleTopologyChangeTimer();
    virtual void clearLocalFDB();
    virtual void clearLocalFDBDelayed();
    virtual bool isBetterThanOwnPrio(MrpPriority remotePrio, MacAddress remoteAddress);
    virtual bool isBetterThanBestPrio(MrpPriority remotePrio, MacAddress remoteAddress);
    virtual void handleTestTimer();
    virtual void handleLinkUpTimer();
    virtual void handleLinkDownTimer();
    virtual void handleContinuityCheckTimer(int ringPort);
    virtual void setupTestRingReq();
    virtual void setupTopologyChangeReq(uint32_t interval);
    virtual void setupContinuityCheck(int ringPort);
    virtual void testRingReq(double time);
    virtual void testMgrNackReq(int ringPort, MrpPriority managerPrio, MacAddress sourceAddress);
    virtual void testPropagateReq(int ringPort, MrpPriority managerPrio, MacAddress sourceAddress);
    virtual void topologyChangeReq(double time);
    virtual void linkChangeReq(int ringPort, uint16_t linkState);
    virtual void setupLinkChangeReq(int ringPort, uint16_t linkState, double time);
    virtual void testRingInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio);
    virtual void topologyChangeInd(MacAddress sourceAddress, double time);
    virtual void linkChangeInd(uint16_t portMode, uint16_t linkState);
    virtual void testMgrNackInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio, MacAddress bestMRMSourceAddress);
    virtual void testPropagateInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio, MacAddress bestMRMSourceAddress, MrpPriority bestMRMPrio);
    virtual void mauTypeChangeInd(int ringPort, uint16_t linkState);
    virtual void interconnTopologyChangeInd(MacAddress sourceAddress, double time, uint16_t inId, int ringPort, Packet* packet);
    virtual void interconnLinkChangeInd(uint16_t inId, uint16_t linkstate, int ringPort, Packet* packet);
    virtual void interconnLinkStatusPollInd(uint16_t inId, int ringPort, Packet* packet);
    virtual void interconnTestInd(MacAddress sourceAddress, int ringPort, uint16_t inId, Packet* packet);
    virtual void interconnForwardReq(int ringPort, Packet* packet);
    virtual void sendFrameReq(int portId, const MacAddress& destinationAddress, const MacAddress& sourceAddress, int prio, uint16_t lt, Packet* MRPPDU);
    virtual void sendCCM(int portId, Packet* CCM);
    virtual void clearFDB(double time);
    virtual void colorLink(NetworkInterface* ie, bool forwarding) const;
    virtual void refreshDisplay() const override;

public:
    Mrp();
    virtual ~Mrp();
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void setTimingProfile(int maxRecoveryTime);
    virtual void setRingInterfaces(int interfaceIndex1, int interfaceIndex2);
    virtual void setRingInterface(int interfaceNumber, int interfaceIndex);
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LAST; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
};

} // namespace inet

#endif

