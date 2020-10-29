//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_ETHERNETSOCKETTABLE_H
#define __INET_ETHERNETSOCKETTABLE_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

class INET_API EthernetSocketTable : public cSimpleModule
{
  public:
    struct Socket
    {
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
    virtual void addSocket(int socketId, MacAddress localAddress, MacAddress remoteAddress, const Protocol *protocol, bool steal);
    virtual void removeSocket(int socketId);
    virtual std::vector<Socket *> findSockets(MacAddress localAddress, MacAddress remoteAddress, const Protocol *protocol) const;
};

} // namespace inet

#endif

