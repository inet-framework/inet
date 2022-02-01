//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetSocketTable.h"

#include "inet/common/stlutils.h"

namespace inet {

Define_Module(EthernetSocketTable);

std::ostream& operator<<(std::ostream& os, const EthernetSocketTable::Socket& socket)
{
    os << "(id:" << socket.socketId << ", local:" << socket.localAddress << ",remote:" << socket.remoteAddress
       << ", prot:" << (socket.protocol ? socket.protocol->getName() : "-") << ")";
    return os;
}

EthernetSocketTable::~EthernetSocketTable()
{
    for (auto it : socketIdToSocketMap)
        delete it.second;
}

void EthernetSocketTable::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        WATCH_PTRMAP(socketIdToSocketMap);
}

void EthernetSocketTable::addSocket(int socketId, MacAddress localAddress, MacAddress remoteAddress, const Protocol *protocol, bool steal)
{
    if (containsKey(socketIdToSocketMap, socketId))
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
    std::vector<EthernetSocketTable::Socket *> result;
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

