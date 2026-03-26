//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4ENCAP_H
#define __INET_IPV4ENCAP_H

#include "inet/common/SimpleModule.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ipv4Encap : public SimpleModule, public IPassivePacketSink
{
  protected:
    struct SocketDescriptor {
        int socketId = -1;
        int protocolId = -1;
        Ipv4Address localAddress;
        Ipv4Address remoteAddress;

        SocketDescriptor(int socketId, int protocolId, Ipv4Address localAddress) : socketId(socketId), protocolId(protocolId), localAddress(localAddress) {}
    };

  protected:
    PassivePacketSinkRef upperLayerSink;
    PassivePacketSinkRef lowerLayerSink;
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;
    int defaultTimeToLive = -1;
    int defaultMCTimeToLive = -1;
    std::set<const Protocol *> upperProtocols;
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleRequest(Request *request);
    virtual void encapsulate(Packet *packet);
    virtual void decapsulate(Packet *packet);

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

