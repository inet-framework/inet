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

#ifndef __INET_IEEE8022LLCSOCKETTABLE_H
#define __INET_IEEE8022LLCSOCKETTABLE_H

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"

namespace inet {

class INET_API Ieee8022LlcSocketTable : public cSimpleModule
{
  public:
    struct Socket
    {
        int socketId = -1;
        int localSap = -1;
        int remoteSap = -1;

        friend std::ostream& operator<<(std::ostream& o, const Socket& t);

        Socket(int socketId) : socketId(socketId) { }
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

