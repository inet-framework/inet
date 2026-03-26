//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022LLCSOCKETIO_H
#define __INET_IEEE8022LLCSOCKETIO_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8022LlcSocketIo : public ApplicationBase, public Ieee8022LlcSocket::ICallback, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef trafficOutSink;
    int localSap = -1;
    int remoteSap = -1;
    MacAddress remoteAddress;
    MacAddress localAddress;
    NetworkInterface *networkInterface = nullptr;

    Ieee8022LlcSocket socket;

    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void setSocketOptions();

    virtual void socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet) override;
    virtual void socketClosed(Ieee8022LlcSocket *socket) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

