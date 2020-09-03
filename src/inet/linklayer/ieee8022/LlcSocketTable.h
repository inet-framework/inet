//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_LLCSOCKETTABLE_H
#define __INET_LLCSOCKETTABLE_H

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"

namespace inet {

class INET_API LlcSocketTable : public cSimpleModule
{
  public:
    struct Socket
    {
        int socketId = -1;
        int localSap = -1;
        int remoteSap = -1;

        friend std::ostream& operator << (std::ostream& o, const Socket& t);

        Socket(int socketId) : socketId(socketId) {}
    };


  protected:
    std::map<int, Socket *> socketIdToSocketMap;

  protected:
    virtual void initialize() override;

  public:
    bool createSocket(int socketId, int localSap, int remoteSap);
    bool deleteSocket(int socketId);
    std::vector<Socket*> findSocketsFor(const Packet *packet) const;
};

} // namespace inet

#endif

