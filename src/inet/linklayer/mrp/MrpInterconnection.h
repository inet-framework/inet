//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MRPINTERCONNECTION_H
#define __INET_MRPINTERCONNECTION_H

#include "Mrp.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/linklayer/mrp/MrpPdu_m.h"

namespace inet {

/**
 * Adds interconnection support to Mrp, i.e. roles MIC and MIM.
 */
class INET_API MrpInterconnection: public Mrp {
protected:
    enum InterconnectionNodeState : uint16_t {
        POWER_ON,
        AC_STAT1, //waiting for the first Link Up at one of its ring ports, starting test monitoring of the ring

        //InterconnectionManager States
        CHK_IO, //Check Interconnection, Interconnection Open State
        CHK_IC, //Check Interconnection, Interconnection Closed State

        //InterConnectionClient States
        PT,  // Pass Through
        IP_IDLE, // Interconnection Port Idle

    };

    enum InterconnectionRole : uint16_t {
        INTERCONNECTION_CLIENT = 1,
        INTERCONNECTION_MANAGER = 2,
    };

    enum InterconnectionTopologyState : uint16_t {
        OPEN = 0x0000,
        CLOSED = 0x0001,
    };

    uint16_t interconnectionID;
    bool linkCheckEnabled = true;  // LC_MODE
    bool ringCheckEnabled = false;  // RC_MODE

    InterconnectionRole inRole = INTERCONNECTION_CLIENT;
    InterconnectionNodeState inNodeState = POWER_ON;
    InterconnectionTopologyState inTopologyState = OPEN;
    uint16_t lastPollId = 0;
    uint16_t lastInTopologyId = 0;
    FrameSentDatabase inTestFrameSent;
    opp_component_ptr<NetworkInterface> interconnectionInterface = nullptr;
    int interconnectionPort;

    simtime_t inLinkChangeInterval;
    simtime_t inTopologyChangeInterval;
    simtime_t inLinkStatusPollInterval;
    simtime_t inTestDefaultInterval;

    uint16_t inTestMonitoringCount = 8; //MRP_IN_TSTNRmax
    uint16_t inTestMaxRetransmissionCount = 0; //MRP_MIM_NRmax
    uint16_t inTestRetransmissionCount = 0; //MRP_MIM_NReturn
    uint16_t inTopologyChangeMaxRepeatCount = 3; //MRP_IN_TOPNRmax
    uint16_t intopologyChangeRepeatCount = 0; //MRP_IN_TC_NReturn
    uint16_t inLinkStatusPollMaxCount = 8; //MRP_IN_LNKSTATNRmax
    uint16_t inLinkStatusPollCount = 0; //MRP_IN_LNKSTAT_NReturn
    uint16_t inLinkMaxChange = 4; //MRP_IN_LNKNRmax
    uint16_t inLinkChangeCount = 0; //MRP_IN_LNKNReturn

    cMessage *inLinkStatusPollTimer = nullptr;
    cMessage *inLinkDownTimer = nullptr;
    cMessage *inLinkUpTimer = nullptr;
    cMessage *inLinkTestTimer = nullptr;
    cMessage *inTopologyChangeTimer = nullptr;

    simsignal_t inLinkChangeSignal;
    simsignal_t inTopologyChangeSignal;
    simsignal_t inStatusPollSignal;
    simsignal_t inTestSignal;
    simsignal_t receivedInChangeSignal;
    simsignal_t receivedInTestSignal;
    simsignal_t receivedInStatusPollSignal;
    simsignal_t interconnectionStateChangedSignal;

protected:
    virtual void start() override;
    virtual void stop() override;
    virtual void initialize(int stage) override;
    InterconnectionRole parseInterconnectionRole(const char *role) const;
    void initInterconnectionPort();
    void micInit();
    void mimInit();
    virtual void handleInTestTimer();
    virtual void handleInLinkStatusPollTimer();
    virtual void handleInTopologyChangeTimer();
    virtual void handleInLinkUpTimer();
    virtual void handleInLinkDownTimer();
    virtual void interconnTestReq(simtime_t time);
    virtual void interconnTopologyChangeReq(simtime_t time);
    virtual void interconnLinkChangeReq(LinkState linkState, simtime_t time);
    virtual void interconnLinkStatusPollReq(simtime_t time);
    virtual void inTransferReq(TlvHeaderType headerType, int ringPort, FrameType frameType, Packet *packet);
    virtual void mrpForwardReq(TlvHeaderType headerType, int ringport, FrameType frameType, Packet *packet);
    virtual void setupInterconnTestReq();
    virtual void setupInterconnTopologyChangeReq(simtime_t time);
    virtual void setupInterconnLinkStatusPollReq();
    virtual void interconnTestInd(MacAddress sourceAddress, int ringPort, uint16_t inId, Packet *packet) override;
    virtual void interconnTopologyChangeInd(MacAddress sourceAddress, simtime_t time, uint16_t inId, int ringPort, Packet *packet) override;
    virtual void interconnLinkChangeInd(uint16_t inId, LinkState linkState, int ringPort, Packet *packet) override;
    virtual void interconnLinkStatusPollInd(uint16_t inId, int ringPort, Packet *packet) override;
    virtual void mauTypeChangeInd(int ringPort, LinkState linkState) override;
    virtual std::string resolveDirective(char directive) const override;
    static const char *getInterconnectionRoleName(InterconnectionRole role, bool acronym);
    static const char *getInterconnectionNodeStateName(InterconnectionNodeState state);
    static const char *getInterconnectionStateName(InterconnectionTopologyState state);

public:
    MrpInterconnection();
    virtual ~MrpInterconnection();
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void setInterconnectionInterface(int InterfaceId);
    virtual void setTimingProfile(int maxRecoveryTime) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LAST; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
};

} // namespace inet

#endif

