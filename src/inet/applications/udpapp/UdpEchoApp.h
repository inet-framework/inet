//
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UDPECHOAPP_H
#define __INET_UDPECHOAPP_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

using namespace inet::queueing;

/**
 * UDP application. See NED for more info.
 */
class INET_API UdpEchoApp : public ApplicationBase, public UdpSocket::ICallback, public IPassivePacketSink, public IModuleInterfaceLookup
{
  protected:
    UdpSocket socket;
    int numEchoed; // just for WATCH

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("upperLayerIn") || gate->isName("lowerLayerIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("upperLayerIn") || gate->isName("lowerLayerIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;
};

} // namespace inet

#endif

