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

#include "inet/linklayer/ethernet/modular/EthernetSocketTable.h"

namespace inet {

Define_Module(EthernetSocketTable);

std::ostream& operator<<(std::ostream& os, const EthernetSocketTable::Socket& socket)
{
    os << "(id:" << socket.socketId << ", local:" << socket.localAddress << ",remote:" << socket.remoteAddress
       << ", prot:" << (socket.protocol ? socket.protocol->getName() : "-") << ")";
    return os;
}

void EthernetSocketTable::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        WATCH_PTRMAP(socketIdToSocketMap);
}

void EthernetSocketTable::addSocket(int socketId, MacAddress localAddress, MacAddress remoteAddress, const Protocol *protocol, bool steal)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end())
        throw cRuntimeError("Socket already added");
    Socket *socket = new Socket(socketId);
    socket->localAddress = localAddress;
    socket->remoteAddress = remoteAddress;
    socket->protocol = protocol;
    socket->steal = steal;
    socketIdToSocketMap[socketId] = socket;
}

void EthernetSocketTable::removeSocket(int socketId)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end()) {
        delete it->second;
        socketIdToSocketMap.erase(it);
    }
    else
        throw cRuntimeError("Socket not found");
}

std::vector<EthernetSocketTable::Socket *> EthernetSocketTable::findSockets(MacAddress localAddress, MacAddress remoteAddress, const Protocol *protocol) const
{
    std::vector<EthernetSocketTable::Socket*> result;
    for (auto& it : socketIdToSocketMap) {
        auto socket = it.second;
        if (!socket->localAddress.isUnspecified() && !localAddress.isBroadcast() && localAddress != socket->localAddress)
            continue;
        if (!socket->remoteAddress.isUnspecified() && !remoteAddress.isBroadcast() && remoteAddress != socket->remoteAddress)
            continue;
        if (socket->protocol != nullptr && protocol != socket->protocol)
            continue;
        result.push_back(socket);
    }
    return result;
}

} // namespace inet

