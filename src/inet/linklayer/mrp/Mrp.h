//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MRP_H
#define __INET_MRP_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/Traced.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ethernet/common/MacForwardingTable.h"
#include "inet/linklayer/mrp/MrpMacForwardingTable.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/linklayer/mrp/MrpInterfaceData.h"
#include "inet/linklayer/mrp/MrpRelay.h"
#include "inet/linklayer/mrp/MrpPdu_m.h"
#include "inet/linklayer/mrp/CfmContinuityCheckMessage_m.h"

namespace inet {

/**
 * Implements the base part of the MRP protocol, i.e. roles MRC, MRM and MRA.
 */
class INET_API Mrp: public OperationalBase, public cListener
{
public:
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

    enum RingState {
        OPEN = 0x0000,
        CLOSED = 0x0001,
        UNDEFINED = 0x0003,
    };

    enum MrpRole {
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

protected:
    typedef std::map<uint16_t, simtime_t> FrameSentDatabase;  // test frame sequence -> time sent
    typedef NetworkInterface::State LinkState;

    const static uint16_t MRP_LT = 0x88E3;

    bool visualize = false;
    Uuid domainID;
    int timingProfile;
    uint16_t vlanID = 0;
    uint8_t priority = 7;
    bool interconnectionLinkCheckAware = true;
    bool interconnectionRingCheckAware = true;
    bool enableLinkCheckOnRing = false;
    cPar *linkDetectionDelayPar;
    cPar *processingDelayPar;

    Traced<MrpRole> role = DISABLED;
    Traced<NodeState> nodeState = POWER_ON;
    Traced<RingState> ringState = OPEN;

    uint16_t transition = 0;
    uint16_t sequenceID = 0;
    uint16_t lastTopologyId = 0;
    MacAddress localBridgeAddress;
    int primaryRingPortId = -1; // interface Id
    int secondaryRingPortId = -1; // interface Id

    //CCM
    int sequenceCCM1 = 0;
    int sequenceCCM2 = 0;

    //Variables need for Manager
    bool addTest = false;
    bool nonblockingMrcSupported = true;  // "Non-blocking MRC supported"
    bool reactOnLinkChange = true;
    bool noTopologyChange = false;  // NO_TC variable
    MrpPriority localManagerPrio = DEFAULT;

    //Variables needed for Automanager
    MacAddress hostBestMRMSourceAddress;
    MrpPriority hostBestMRMPriority;
    uint16_t monNReturn;
    uint16_t monNRmax = 5;
    cMessage *linkUpHysteresisTimer = nullptr; //LNKUP_HYST_TIMER_RUNNING

    simtime_t linkDownInterval = SimTime(20, SIMTIME_MS); //MRP_LNKdownT
    simtime_t linkUpInterval = SimTime(20, SIMTIME_MS); //MRP_LNKupT
    simtime_t topologyChangeInterval; //MRP_TOPchgT
    simtime_t shortTestInterval; //MRP_TSTshortT
    simtime_t defaultTestInterval; //MRP_TSTdefaultT
    simtime_t ccmInterval = SimTime(10, SIMTIME_MS); // time in milliseconds. either 3.3 or 10 ms

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

    simsignal_t ringPort1StateChangedSignal;
    simsignal_t ringPort2StateChangedSignal;
    simsignal_t topologyChangeAnnouncedSignal;
    simsignal_t fdbClearedSignal;
    simsignal_t linkChangeDetectedSignal;
    simsignal_t testFrameLatencySignal;

public:
    typedef MrpInterfaceData::PortInfo PortInfo;

protected:
    virtual void start();
    virtual void stop();
    virtual void initialize(int stage) override;
    virtual MrpRole parseMrpRole(const char *mrpRole) const;
    virtual void mrcInit();
    virtual void mrmInit();
    virtual void mraInit();
    virtual void initPortTable();
    virtual void initRingPort(int interfaceId, MrpInterfaceData::PortRole role, bool enableLinkCheck);
    virtual void startContinuityCheck();
    virtual void initInterfacedata(int interfaceId);
    virtual void toggleRingPorts();
    virtual void setPortRole(int interfaceId, MrpInterfaceData::PortRole role);
    virtual void setPortState(int interfaceId, MrpInterfaceData::PortState state);
    virtual MrpInterfaceData::PortState getPortState(int interfaceId) const;
    virtual MrpInterfaceData::PortRole getPortRole(int interfaceId) const;
    virtual NetworkInterface* getPortNetworkInterface(int interfaceId) const;
    virtual MrpInterfaceData* getPortInterfaceDataForUpdate(int interfaceId);
    virtual const MrpInterfaceData* getPortInterfaceData(int interfaceId) const;
    virtual void handleMrpPDU(Packet* packet);
    virtual void handleCfmContinuityCheckMessage(Packet* packet);
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
    virtual void setupTopologyChangeReq(simtime_t interval);
    virtual void setupContinuityCheck(int ringPort);
    virtual void testRingReq(simtime_t time);
    virtual void testMgrNackReq(int ringPort, MrpPriority managerPrio, MacAddress sourceAddress);
    virtual void testPropagateReq(int ringPort, MrpPriority managerPrio, MacAddress sourceAddress);
    virtual void topologyChangeReq(simtime_t time);
    virtual void linkChangeReq(int ringPort, LinkState linkState);
    virtual void setupLinkChangeReq(int ringPort, LinkState linkState, simtime_t time);
    virtual void testRingInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio);
    virtual void topologyChangeInd(MacAddress sourceAddress, simtime_t time);
    virtual void linkChangeInd(LinkState linkState);
    virtual void testMgrNackInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio, MacAddress bestMRMSourceAddress);
    virtual void testPropagateInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio, MacAddress bestMRMSourceAddress, MrpPriority bestMRMPrio);
    virtual void mauTypeChangeInd(int ringPort, LinkState linkState);
    virtual void interconnTopologyChangeInd(MacAddress sourceAddress, simtime_t time, uint16_t inId, int ringPort, Packet* packet);
    virtual void interconnLinkChangeInd(uint16_t inId, LinkState linkState, int ringPort, Packet* packet);
    virtual void interconnLinkStatusPollInd(uint16_t inId, int ringPort, Packet* packet);
    virtual void interconnTestInd(MacAddress sourceAddress, int ringPort, uint16_t inId, Packet* packet);
    virtual void interconnForwardReq(int ringPort, Packet* packet);
    virtual void sendFrameReq(int portId, const MacAddress& destinationAddress, const MacAddress& sourceAddress, int prio, uint16_t lt, Packet* MRPPDU);
    virtual void sendCCM(int portId, Packet* ccm);
    virtual void clearFDB(simtime_t time);
    virtual void colorLink(NetworkInterface* ie, bool forwarding) const;
    virtual void refreshDisplay() const override;
    virtual std::string resolveDirective(char directive) const override;
    static const char *getMrpRoleName(MrpRole role, bool acronym);
    static const char *getNodeStateName(NodeState state);
    static const char *getRingStateName(RingState state);

public:
    Mrp();
    virtual ~Mrp();
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void setTimingProfile(int maxRecoveryTime);
    virtual int resolveInterfaceIndex(int interfaceIndex);
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LAST; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
};

} // namespace inet

#endif

