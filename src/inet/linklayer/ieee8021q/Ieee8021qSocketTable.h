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

namespace inet {

class INET_API Ieee8021qSocketTable : public cSimpleModule
{
  public:
    struct Socket
    {
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
    virtual void addSocket(int socketId, const Protocol *protocol, int vlanId, bool steal);
    virtual void removeSocket(int socketId);
    virtual std::vector<Socket *> findSockets(const Protocol *protocol, int vlanId) const;
};

} // namespace inet

#endif

