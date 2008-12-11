//
// Copyright (C) 2004 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <omnetpp.h>
#include "TCPSocketMap.h"


TCPSocket *TCPSocketMap::findSocketFor(cMessage *msg)
{
    TCPCommand *ind = dynamic_cast<TCPCommand *>(msg->getControlInfo());
    if (!ind)
        opp_error("TCPSocketMap: findSocketFor(): no TCPCommand control info in message (not from TCP?)");
    int connId = ind->getConnId();
    SocketMap::iterator i = socketMap.find(connId);
    ASSERT(i==socketMap.end() || i->first==i->second->getConnectionId());
    return (i==socketMap.end()) ? NULL : i->second;
}

void TCPSocketMap::addSocket(TCPSocket *socket)
{
    ASSERT(socketMap.find(socket->getConnectionId())==socketMap.end());
    socketMap[socket->getConnectionId()] = socket;
}

TCPSocket *TCPSocketMap::removeSocket(TCPSocket *socket)
{
    SocketMap::iterator i = socketMap.find(socket->getConnectionId());
    if (i!=socketMap.end())
        socketMap.erase(i);
    return socket;
}

void TCPSocketMap::deleteSockets()
{
    for (SocketMap::iterator i=socketMap.begin(); i!=socketMap.end(); ++i)
       delete i->second;
}
