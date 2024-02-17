// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_INTERCONNECTIONNODE_H
#define __INET_INTERCONNECTIONNODE_H

#include "../mediaredundancynode/MediaRedundancyNode.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
//#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/linklayer/mrp/common/MrpPdu_m.h"

namespace inet {

/**
 * Base class for MIC and MIM.
 */
class INET_API InterconnectionNode: public MediaRedundancyNode {
protected:
    enum inNodeState : uint16_t {
        POWER_ON, AC_STAT1, //waiting for the first Link Up at one of its ring ports, starting test monitoring of the ring

        //InterconnectionManager States
        CHK_IO, //Check Interconnection, Interconnection Open State
        CHK_IC, //Check Interconnection, Interconnection Closed State

        //InterConnectionClient States
        PT,
        IP_IDLE,
    };

    enum inRoleState : uint16_t {
        INTERCONNECTION_CLIENT = 1, INTERCONNECTION_MANAGER = 2,
    };

    enum interConnectionState : uint16_t {
        OPEN = 0x0000, CLOSED = 0x0001,
    };

    uint16_t interConnectionID;
    std::string interconnectionName;
    bool linkCheckEnabled = true;
    bool ringCheckEnabled = false;
    inRoleState inRole = INTERCONNECTION_CLIENT;

    inNodeState inState = POWER_ON;
    interConnectionState currentInterconnectionState = OPEN;
    uint16_t lastPollId = 0;
    uint16_t lastInTopologyId = 0;
    FrameSentDatabase inTestFrameSent;
    opp_component_ptr<NetworkInterface> interconnectionInterface = nullptr;
    int interconnectionPort = 2;

    double inLinkChangeInterval;
    double inTopologyChangeInterval;
    double inLinkStatusPollInterval;
    double inTestDefaultInterval;

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

    simsignal_t InLinkChangeSignal;
    simsignal_t InTopologyChangeSignal;
    simsignal_t InStatusPollSignal;
    simsignal_t InTestSignal;
    simsignal_t ReceivedInChangeSignal;
    simsignal_t ReceivedInTestSignal;
    simsignal_t ReceivedInStatusPollSignal;
    simsignal_t InterconnectionStateChangedSignal;

protected:
    virtual void start() override;
    virtual void stop() override;
    virtual void read() override;
    virtual void initialize(int stage) override;
    void initInterconnectionPort();
    void micInit();
    void mimInit();
    virtual void handleInTestTimer();
    virtual void handleInLinkStatusPollTimer();
    virtual void handleInTopologyChangeTimer();
    virtual void handleInLinkUpTimer();
    virtual void handleInLinkDownTimer();
    virtual void interconnTestReq(double Time);
    virtual void interconnTopologyChangeReq(double Time);
    virtual void interconnLinkChangeReq(uint16_t LinkState, double Time);
    virtual void interconnLinkStatusPollReq(double Time);
    virtual void inTransferReq(tlvHeaderType HeaderType, int RingPort, frameType FrameType, Packet *packet);
    virtual void mrpForwardReq(tlvHeaderType HeaderType, int Ringport, frameType FrameType, Packet *packet);
    virtual void setupInterconnTestReq();
    virtual void setupInterconnTopologyChangeReq(double Time);
    virtual void setupInterconnLinkStatusPollReq();
    virtual void interconnTestInd(MacAddress SourceAddress, int RingPort, uint16_t InID, Packet *packet) override;
    virtual void interconnTopologyChangeInd(MacAddress SourceAddress, double Time, uint16_t InID, int RingPort, Packet *packet) override;
    virtual void interconnLinkChangeInd(uint16_t InID, uint16_t LinkState, int RingPort, Packet *packet) override;
    virtual void interconnLinkStatusPollInd(uint16_t InID, int RingPort, Packet *packet) override;
    virtual void mauTypeChangeInd(int RingPort, uint16_t LinkState) override;

public:
    InterconnectionNode();
    virtual ~InterconnectionNode();
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

