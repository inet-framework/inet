//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ETHERTRAFGEN_H
#define __INET_ETHERTRAFGEN_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

/**
 * Simple traffic generator for the Ethernet model.
 */
class INET_API EtherTrafGen : public ApplicationBase
{
  protected:
    enum Kinds { START = 100, NEXT };

    ModuleRefByPar<IInterfaceTable> interfaceTable;

    long seqNum = 0;

    // send parameters
    cPar *sendInterval = nullptr;
    cPar *numPacketsPerBurst = nullptr;
    cPar *packetLength = nullptr;
    int ssap = -1;
    int dsap = -1;
    MacAddress destMacAddress;
    int outInterface = -1;

    Ieee8022LlcSocket llcSocket;
    // self messages
    cMessage *timerMsg = nullptr;
    simtime_t startTime;
    simtime_t stopTime;

    // receive statistics
    long packetsSent = 0;
    long packetsReceived = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual bool isGenerator();
    virtual void scheduleNextPacket(simtime_t previous);
    virtual void cancelNextPacket();

    virtual MacAddress resolveDestMacAddress();

    virtual void sendBurstPackets();
    virtual void receivePacket(Packet *msg);

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    EtherTrafGen();
    virtual ~EtherTrafGen();
};

} // namespace inet

#endif

