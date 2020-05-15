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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"
#include "inet/protocol/ethernet/LlcSocketTable.h"

namespace inet {

Define_Module(LlcSocketTable);

std::ostream& operator << (std::ostream& o, const LlcSocketTable::Socket& t)
{
    o << "(id:" << t.socketId << ",lsap:" << t.localSap << ",rsap" << t.remoteSap << ")";
    return o;
}

void LlcSocketTable::initialize()
{
    WATCH_PTRMAP(socketIdToSocketMap);
}

bool LlcSocketTable::createSocket(int socketId, int localSap, int remoteSap)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end())
        return false;

    Socket *socket = new Socket(socketId);
    socket->localSap = localSap;
    socket->remoteSap = remoteSap;
    socketIdToSocketMap[socketId] = socket;
    return true;
}

bool LlcSocketTable::deleteSocket(int socketId)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it == socketIdToSocketMap.end())
        return false;

    delete it->second;
    socketIdToSocketMap.erase(it);
    return true;
}

std::vector<LlcSocketTable::Socket*> LlcSocketTable::findSocketsFor(const Packet *packet) const
{
    std::vector<LlcSocketTable::Socket*> retval;
    if (auto sap = packet->findTag<Ieee802SapInd>()) {
        int localSap = sap->getDsap();
        int remoteSap = sap->getSsap();

        for (auto s : socketIdToSocketMap) {
            auto socket = s.second;
            if ((socket->localSap == localSap || socket->localSap == -1)
                    && (socket->remoteSap == remoteSap || socket->remoteSap == -1)) {
                retval.push_back(socket);
            }
        }
    }
    return retval;
}

} // namespace inet

