//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPSOCKETIO_H
#define __INET_IPSOCKETIO_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/networklayer/contract/ipv4/Ipv4Socket.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

class INET_API IpSocketIo : public ApplicationBase, public Ipv4Socket::ICallback, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef trafficOutSink;
    const Protocol *protocol = nullptr;
    bool dontFragment = false;
    Ipv4Socket socket;
    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void setSocketOptions();

    virtual void socketDataArrived(Ipv4Socket *socket, Packet *packet) override;
    virtual void socketClosed(Ipv4Socket *socket) override;

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

