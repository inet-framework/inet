//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4SocketTable.h"

#include "inet/common/stlutils.h"

namespace inet {

Define_Module(Ipv4SocketTable);

std::ostream& operator<<(std::ostream& os, const Ipv4SocketTable::Socket& socket)
{
    auto protocol = Protocol::getProtocol(socket.protocolId);
    os << "(id:" << socket.socketId << ", local:" << socket.localAddress << ",remote:" << socket.remoteAddress
       << ", prot:" << (protocol != nullptr ? protocol->getName() : "-") << ")";
    return os;
}

void Ipv4SocketTable::clearSockets()
{
    for (auto& it : socketIdToSocketMap)
        delete it.second;

    socketIdToSocketMap.clear();
}

Ipv4SocketTable::~Ipv4SocketTable()
{
    clearSockets();
}

void Ipv4SocketTable::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
        WATCH_PTRMAP(socketIdToSocketMap);
}

void Ipv4SocketTable::addSocket(int socketId, int protocolId, Ipv4Address localAddress)
{
    if (containsKey(socketIdToSocketMap, socketId))
        throw cRuntimeError("Socket already added");
    Socket *socket = new Socket(socketId);
    socket->localAddress = localAddress;
    socket->protocolId = protocolId;
    socketIdToSocketMap[socketId] = socket;
}

bool Ipv4SocketTable::connectSocket(int socketId, Ipv4Address remoteAddress)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end()) {
        it->second->remoteAddress = remoteAddress;
        return true;
    }
    else
        return false;
}

bool Ipv4SocketTable::removeSocket(int socketId)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it != socketIdToSocketMap.end()) {
        delete it->second;
        socketIdToSocketMap.erase(it);
        return true;
    }
    else
        return false;
}

std::vector<Ipv4SocketTable::Socket *> Ipv4SocketTable::findSockets(Ipv4Address localAddress, Ipv4Address remoteAddress, int protocolId) const
{
    std::vector<Ipv4SocketTable::Socket *> result;
    for (const auto& elem : socketIdToSocketMap) {
        auto socket = elem.second;
        if (socket->protocolId == protocolId
                && (socket->localAddress.isUnspecified() || socket->localAddress == localAddress)
                && (socket->remoteAddress.isUnspecified() || socket->remoteAddress == remoteAddress))
        {
            result.push_back(socket);
        }
    }
    return result;
}

} // namespace inet
