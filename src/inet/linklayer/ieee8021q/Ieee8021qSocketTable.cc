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

#include "inet/linklayer/ieee8021q/Ieee8021qSocketTable.h"

namespace inet {

Define_Module(Ieee8021qSocketTable);

std::ostream& operator<<(std::ostream& os, const Ieee8021qSocketTable::Socket& socket)
{
    os << "(id = " << socket.socketId << ", protocol = " << (socket.protocol ? socket.protocol->getName() : "-") << ", vlanId = " << socket.vlanId << ")";
    return os;
}

void Ieee8021qSocketTable::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        WATCH_PTRMAP(socketIdToSocketMap);
}

void Ieee8021qSocketTable::addSocket(int socketId, const Protocol *protocol, int vlanId, bool steal)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end())
        throw cRuntimeError("Socket already added");
    Socket *socket = new Socket(socketId);
    socket->protocol = protocol;
    socket->vlanId = vlanId;
    socket->steal = steal;
    socketIdToSocketMap[socketId] = socket;
}

void Ieee8021qSocketTable::removeSocket(int socketId)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end()) {
        delete it->second;
        socketIdToSocketMap.erase(it);
    }
    else
        throw cRuntimeError("Socket not found");
}

std::vector<Ieee8021qSocketTable::Socket *> Ieee8021qSocketTable::findSockets(const Protocol *protocol, int vlanId) const
{
    std::vector<Ieee8021qSocketTable::Socket*> result;
    for (auto& it : socketIdToSocketMap) {
        auto socket = it.second;
        if (socket->protocol != nullptr && protocol != socket->protocol)
            continue;
        if (socket->vlanId != -1 && vlanId != socket->vlanId)
            continue;
        result.push_back(socket);
    }
    return result;
}

} // namespace inet

