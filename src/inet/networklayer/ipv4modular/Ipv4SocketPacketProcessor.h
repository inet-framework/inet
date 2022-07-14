//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4SOCKETPACKETPROCESSOR_H
#define __INET_IPV4SOCKETPACKETPROCESSOR_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/networklayer/ipv4modular/Ipv4SocketTable.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ipv4SocketPacketProcessor : public queueing::PacketPusherBase, public TransparentProtocolRegistrationListener
{
  protected:
    ModuleRefByPar<Icmp> icmp;
    ModuleRefByPar<Ipv4SocketTable> socketTable;
    std::set<const Protocol *> upperProtocols; // where to send packets after decapsulation

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;


  public:
    virtual void pushPacket(Packet *packet, cGate *arrivalGate) override;
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual void processPushCommand(Message *message, cGate *arrivalGate);
};

} // namespace inet

#endif
