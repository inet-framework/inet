//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4ENCAP_H
#define __INET_IPV4ENCAP_H

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

class INET_API Ipv4Encap : public cSimpleModule
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
    CrcMode crcMode = CRC_MODE_UNDEFINED;
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
};

} // namespace inet

#endif

