//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETSOCKETTABLE_H
#define __INET_ETHERNETSOCKETTABLE_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

class INET_API EthernetSocketTable : public cSimpleModule
{
  public:
    struct Socket {
        int socketId = -1;
        MacAddress localAddress;
        MacAddress remoteAddress;
        const Protocol *protocol = nullptr;
        bool steal = false;

        Socket(int socketId) : socketId(socketId) {}

        friend std::ostream& operator<<(std::ostream& o, const Socket& t);
    };

  protected:
    std::map<int, Socket *> socketIdToSocketMap;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual ~EthernetSocketTable();

    virtual void addSocket(int socketId, MacAddress localAddress, MacAddress remoteAddress, const Protocol *protocol, bool steal);
    virtual void removeSocket(int socketId);
    virtual std::vector<Socket *> findSockets(MacAddress localAddress, MacAddress remoteAddress, const Protocol *protocol) const;
};

} // namespace inet

#endif

