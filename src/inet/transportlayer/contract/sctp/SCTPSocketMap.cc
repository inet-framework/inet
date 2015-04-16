//
// Copyright (C) 2015 Thomas Dreibholz (dreibh@simula.no)
// Based on TCPSocketMap.cc, Copyright (C) 2004 Andras Varga
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

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/contract/sctp/SCTPSocketMap.h"

namespace inet {

SCTPSocket *SCTPSocketMap::findSocketFor(cMessage *msg)
{
    SCTPCommand *ind = dynamic_cast<SCTPCommand *>(msg->getControlInfo());
    if (!ind)
        throw cRuntimeError("SCTPSocketMap: findSocketFor(): no SCTPCommand control info in message (not from SCTP?)");

//     int connId = ind->getConnId();
puts("FIXME!");
int connId=-1;    
::abort();    

    auto i = socketMap.find(connId);
    ASSERT(i == socketMap.end() || i->first == i->second->getConnectionId());
    return (i == socketMap.end()) ? nullptr : i->second;
}

void SCTPSocketMap::addSocket(SCTPSocket *socket)
{
    ASSERT(socketMap.find(socket->getConnectionId()) == socketMap.end());
    socketMap[socket->getConnectionId()] = socket;
}

SCTPSocket *SCTPSocketMap::removeSocket(SCTPSocket *socket)
{
    auto i = socketMap.find(socket->getConnectionId());
    if (i != socketMap.end())
        socketMap.erase(i);
    return socket;
}

void SCTPSocketMap::deleteSockets()
{
    for (auto i = socketMap.begin(); i != socketMap.end(); ++i)
        delete i->second;
    socketMap.clear();
}

} // namespace inet
