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

#include "inet/linklayer/ieee8022/Ieee8022LlcSocketTable.h"

namespace inet {

Define_Module(Ieee8022LlcSocketTable);

std::ostream& operator << (std::ostream& o, const Ieee8022LlcSocketTable::Socket& t)
{
    o << "(id:" << t.socketId << ",lsap:" << t.localSap << ",rsap" << t.remoteSap << ")";
    return o;
}

void Ieee8022LlcSocketTable::initialize()
{
    WATCH_PTRMAP(socketIdToSocketMap);
}

void Ieee8022LlcSocketTable::addSocket(int socketId, int localSap, int remoteSap)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end())
        throw cRuntimeError("Socket already added");
    Socket *socket = new Socket(socketId);
    socket->localSap = localSap;
    socket->remoteSap = remoteSap;
    socketIdToSocketMap[socketId] = socket;
}

void Ieee8022LlcSocketTable::removeSocket(int socketId)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end()) {
        delete it->second;
        socketIdToSocketMap.erase(it);
    }
    else
        throw cRuntimeError("Socket not found");
}

std::vector<Ieee8022LlcSocketTable::Socket *> Ieee8022LlcSocketTable::findSockets(int localSap, int remoteSap) const
{
    std::vector<Ieee8022LlcSocketTable::Socket*> result;
    for (auto& it : socketIdToSocketMap) {
        auto socket = it.second;
        if ((socket->localSap == localSap || socket->localSap == -1) &&
            (socket->remoteSap == remoteSap || socket->remoteSap == -1))
        {
            result.push_back(socket);
        }
    }
    return result;
}

} // namespace inet

