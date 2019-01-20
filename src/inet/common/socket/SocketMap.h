//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_ISOCKETMAP_H
#define __INET_ISOCKETMAP_H

#include <map>

#include "inet/common/INETDefs.h"
#include "inet/common/socket/ISocket.h"

namespace inet {

/**
 * Small utility class for managing a large number of ISocket objects.
 */
class INET_API SocketMap
{
  protected:
    std::map<int, ISocket *> socketMap;         // socketId, socketPtr

  public:
    SocketMap() {}

    /**
     * Destructor does NOT delete the socket objects.
     */
    ~SocketMap() {}

    /**
     * Finds the socket for the given message.
     */
    ISocket *findSocketFor(cMessage *msg);

    /**
     * Adds the given socket.
     */
    void addSocket(ISocket *socket);

    /**
     * Removes the given socket.
     * Returns removed socket or returns nullptr when socket not found in the map
     */
    ISocket *removeSocket(ISocket *socket);

    /**
     * Returns the number of sockets stored.
     */
    unsigned int size() const { return socketMap.size(); }

    /**
     * Returns the socket map
     */
    std::map<int, ISocket *>& getMap() { return socketMap; }

    /**
     * Returns the socket specified by socketId
     */
    ISocket *getSocketById(unsigned int socketId) { return socketMap.at(socketId); }

    /**
     * Deletes the socket objects.
     */
    void deleteSockets();

    void addWatch();
};

std::ostream& operator<<(std::ostream& out, const ISocket& entry);

} // namespace inet

#endif // ifndef __INET_ISOCKETMAP_H

