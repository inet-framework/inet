//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ETHERAPPCLIENT_H
#define __INET_ETHERAPPCLIENT_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace inet::queueing;

/**
 * Simple traffic generator for the Ethernet model.
 */
class INET_API EtherAppClient : public ApplicationBase, public Ieee8022LlcSocket::ICallback, public IPassivePacketSink, public IModuleInterfaceLookup
{
  protected:
    enum Kinds { START = 100, NEXT };

    // send parameters
    long seqNum = 0;
    cPar *reqLength = nullptr;
    cPar *respLength = nullptr;
    cPar *sendInterval = nullptr;

    int localSap = -1;
    int remoteSap = -1;
    MacAddress destMacAddress;
    ModuleRefByPar<IInterfaceTable> interfaceTable;

    // self messages
    cMessage *timerMsg = nullptr;
    simtime_t startTime;
    simtime_t stopTime;

    Ieee8022LlcSocket llcSocket;

    // receive statistics
    long packetsSent = 0;
    long packetsReceived = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual bool isGenerator();
    virtual void scheduleNextPacket(bool start);
    virtual void cancelNextPacket();

    virtual MacAddress resolveDestMacAddress();

    virtual void sendPacket();

    // Ieee8022LlcSocket::ICallback
    virtual void socketDataArrived(Ieee8022LlcSocket *, Packet *msg) override;
    virtual void socketClosed(Ieee8022LlcSocket *socket) override;

    // ApplicationBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    EtherAppClient() {}
    virtual ~EtherAppClient();

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("in"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("in"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;
};

} // namespace inet

#endif

