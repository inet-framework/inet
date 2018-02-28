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

#include "inet/transportlayer/contract/sctp/SctpSocketMap.h"

namespace inet {

SctpSocket *SctpSocketMap::findSocketFor(cMessage *msg)
{
    auto& tags = getTags(msg);
    SctpCommandReq *ind = tags.findTag<SctpCommandReq>();
    if (!ind)
        throw cRuntimeError("SctpSocketMap: findSocketFor(): no SctpCommand control info in message (not from SCTP?)");

    for (auto & elem : socketMap) {
        if (elem.second->belongsToSocket(msg)) {
            return elem.second;
        }
    }
    return nullptr;
}

void SctpSocketMap::addSocket(SctpSocket *socket)
{
    ASSERT(socketMap.find(socket->getConnectionId()) == socketMap.end());
    socketMap[socket->getConnectionId()] = socket;
}

SctpSocket *SctpSocketMap::removeSocket(SctpSocket *socket)
{
    auto i = socketMap.find(socket->getConnectionId());
    if (i != socketMap.end())
        socketMap.erase(i);
    return socket;
}

void SctpSocketMap::deleteSockets()
{
    for (auto & elem : socketMap)
        delete elem.second;
    socketMap.clear();
}

} // namespace inet
