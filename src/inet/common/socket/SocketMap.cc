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

