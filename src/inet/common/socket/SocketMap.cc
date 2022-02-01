//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/socket/SocketMap.h"

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

ISocket *SocketMap::findSocketFor(cMessage *msg)
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    int connId = tags.getTag<SocketInd>()->getSocketId();
    auto i = socketMap.find(connId);
    ASSERT(i == socketMap.end() || i->first == i->second->getSocketId());
    return (i == socketMap.end()) ? nullptr : i->second;
}

void SocketMap::addSocket(ISocket *socket)
{
    ASSERT(!containsKey(socketMap, socket->getSocketId()));
    socketMap[socket->getSocketId()] = socket;
}

ISocket *SocketMap::removeSocket(ISocket *socket)
{
    auto i = socketMap.find(socket->getSocketId());
    if (i != socketMap.end()) {
        socketMap.erase(i);
        return socket;
    }
    return nullptr;
}

void SocketMap::deleteSockets()
{
    for (auto& elem : socketMap)
        delete elem.second;
    socketMap.clear();
}

void SocketMap::addWatch()
{
    WATCH_PTRMAP(socketMap);
}

std::ostream& operator<<(std::ostream& out, const ISocket& entry)
{
    const UdpSocket *udp = dynamic_cast<const UdpSocket *>(&entry);
    if (udp) {
        out << "UDPConnectionId: " << udp->getSocketId();
    }

    const TcpSocket *tcp = dynamic_cast<const TcpSocket *>(&entry);
    if (tcp) {
        out << "TCPConnectionId: " << tcp->getSocketId() << " "
            << " local: " << tcp->getLocalAddress() << ":" << tcp->getLocalPort() << " "
            << " remote: " << tcp->getRemoteAddress() << ":" << tcp->getRemotePort() << " "
            << " status: " << TcpSocket::stateName(tcp->getState());
    }

    return out;
}

} // namespace inet

