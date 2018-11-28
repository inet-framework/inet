//
// Copyright (C) OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IPV4ENCAP_H
#define __INET_IPV4ENCAP_H

#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

class INET_API Ipv4Encap : public cSimpleModule
{
  protected:
    struct SocketDescriptor
    {
        int socketId = -1;
        int protocolId = -1;
        Ipv4Address localAddress;
        Ipv4Address remoteAddress;

        SocketDescriptor(int socketId, int protocolId, Ipv4Address localAddress) : socketId(socketId), protocolId(protocolId), localAddress(localAddress) { }
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

#endif // ifndef __INET_IPV4ENCAP_H

