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

#ifndef __INET_TCPSOCKETMAP_H
#define __INET_TCPSOCKETMAP_H

#include <map>

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

/**
 * Small utility class for managing a large number of TcpSocket objects.
 */
//TBD: need for lookup could be eliminated by adding a void *yourPtr into TcpConnection and TcpCommand
class INET_API TcpSocketMap
{
  protected:
    typedef std::map<int, TcpSocket *> SocketMap;
    SocketMap socketMap;

  public:
    /**
     * Constructor.
     */
    TcpSocketMap() {}

    /**
     * Destructor. Does NOT delete the TcpSocket objects.
     */
    ~TcpSocketMap() {}

    /**
     * Finds the socket (by connId) for the given message. The message
     * must have arrived from TCP, and must contain a TcpCommand
     * control info object. The method returns nullptr if the socket was
     * not found, and throws an error if the message doesn't contain
     * a TcpCommand.
     */
    TcpSocket *findSocketFor(cMessage *msg);

    /**
     * Registers the given socket. Should not be called multiple times
     * for one socket object.
     */
    void addSocket(TcpSocket *socket);

    /**
     * Removes the given socket from the data structure.
     */
    TcpSocket *removeSocket(TcpSocket *socket);

    /**
     * Returns the number of sockets stored.
     */
    unsigned int size() const { return socketMap.size(); }

    /**
     * Deletes the socket objects.
     */
    void deleteSockets();
};

} // namespace inet

#endif // ifndef __INET_TCPSOCKETMAP_H

