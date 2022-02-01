//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022LLCSOCKETTABLE_H
#define __INET_IEEE8022LLCSOCKETTABLE_H

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"

namespace inet {

class INET_API Ieee8022LlcSocketTable : public cSimpleModule
{
  public:
    struct Socket {
        int socketId = -1;
        int localSap = -1;
        int remoteSap = -1;

        friend std::ostream& operator<<(std::ostream& o, const Socket& t);

        Socket(int socketId) : socketId(socketId) {}
    };

  protected:
    std::map<int, Socket *> socketIdToSocketMap;

  protected:
    virtual void initialize() override;

  public:
    void addSocket(int socketId, int localSap, int remoteSap);
    void removeSocket(int socketId);
    std::vector<Socket *> findSockets(int localSap, int remoteSap) const;
};

} // namespace inet

#endif

