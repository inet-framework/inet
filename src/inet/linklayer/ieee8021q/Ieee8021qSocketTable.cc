//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qSocketTable.h"

#include "inet/common/stlutils.h"

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

Ieee8021qSocketTable::~Ieee8021qSocketTable()
{
    for (auto it : socketIdToSocketMap)
        delete it.second;
}

void Ieee8021qSocketTable::addSocket(int socketId, const Protocol *protocol, int vlanId, bool steal)
{
    if (containsKey(socketIdToSocketMap, socketId))
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
    std::vector<Ieee8021qSocketTable::Socket *> result;
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

