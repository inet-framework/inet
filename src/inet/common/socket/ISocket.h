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

#ifndef __INET_ISOCKET_H
#define __INET_ISOCKET_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class provides an interface that should be implemented by all sockets.
 */
class INET_API ISocket
{
  public:
    virtual ~ISocket() {}

    /**
     * Returns the socket Id which is unique within the network node.
     */
    virtual int getSocketId() const = 0;

    /**
     * Returns true if the message belongs to this socket.
     */
    virtual bool belongsToSocket(cMessage *msg) const = 0;

    /**
     * Examines the message, takes ownership, and updates socket state.
     */
    virtual void processMessage(cMessage *msg) = 0;

    /**
     * Close the socket.
     */
    virtual void close() = 0;

    /**
     * Notify the protocol that the owner of ISocket has destroyed the socket.
     * Typically used when the owner of ISocket has crashed.
     */
    virtual void destroy() = 0;

    virtual bool isOpen() const = 0;
};

} // namespace inet

#endif // ifndef __INET_ISOCKET_H

