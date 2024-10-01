//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETSOCKETIO_H
#define __INET_ETHERNETSOCKETIO_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/Protocol.h"
#include "inet/linklayer/ethernet/common/EthernetSocket.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetSocketIo : public ApplicationBase, public EthernetSocket::ICallback, public IPassivePacketSink, public IModuleInterfaceLookup
{
  protected:
    PassivePacketSinkRef trafficSink;
    NetworkInterface *networkInterface = nullptr;
    const Protocol *protocol = nullptr;
    MacAddress remoteAddress;
    MacAddress localAddress;
    EthernetSocket socket;
    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void setSocketOptions();

    virtual void socketDataArrived(EthernetSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(EthernetSocket *socket, Indication *indication) override;
    virtual void socketClosed(EthernetSocket *socket) override;

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("trafficIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("trafficIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual cGate* lookupModuleInterface(cGate* gate, const std::type_info& type, const cObject* arguments, int direction) override;
};

} // namespace inet

#endif

