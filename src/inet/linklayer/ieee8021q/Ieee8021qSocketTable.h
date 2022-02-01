//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021QSOCKETTABLE_H
#define __INET_IEEE8021QSOCKETTABLE_H

#include "inet/common/Protocol.h"

namespace inet {

class INET_API Ieee8021qSocketTable : public cSimpleModule
{
  public:
    struct Socket {
        int socketId = -1;
        const Protocol *protocol = nullptr;
        int vlanId = -1;
        bool steal = false;

        Socket(int socketId) : socketId(socketId) {}

        friend std::ostream& operator<<(std::ostream& o, const Socket& t);
    };

  protected:
    std::map<int, Socket *> socketIdToSocketMap;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual ~Ieee8021qSocketTable();

    virtual void addSocket(int socketId, const Protocol *protocol, int vlanId, bool steal);
    virtual void removeSocket(int socketId);
    virtual std::vector<Socket *> findSockets(const Protocol *protocol, int vlanId) const;
};

} // namespace inet

#endif

