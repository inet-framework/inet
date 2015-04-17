//
// Copyright (C) 2004 Andras Varga
// Based on SCTPSocketMap.h, Copyright (C) 2004 Andras Varga
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

#ifndef __INET_SCTPSOCKETMAP_H
#define __INET_SCTPSOCKETMAP_H

#include <map>

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/contract/sctp/SCTPSocket.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"

namespace inet {

/**
 * Small utility class for managing a large number of SCTPSocket objects.
 */
//TBD: need for lookup could be eliminated by adding a void *yourPtr into SCTPConnection and SCTPCommand
class INET_API SCTPSocketMap
{
  protected:
    typedef std::map<int, SCTPSocket *> SocketMap;
    SocketMap socketMap;

  public:
    /**
     * Constructor.
     */
    SCTPSocketMap() {}

    /**
     * Destructor. Does NOT delete the SCTPSocket objects.
     */
    ~SCTPSocketMap() {}

    /**
     * Finds the socket (by connId) for the given message. The message
     * must have arrived from SCTP, and must contain a SCTPCommand
     * control info object. The method returns nullptr if the socket was
     * not found, and throws an error if the message doesn't contain
     * a SCTPCommand.
     */
    SCTPSocket *findSocketFor(cMessage *msg);

    /**
     * Registers the given socket. Should not be called multiple times
     * for one socket object.
     */
    void addSocket(SCTPSocket *socket);

    /**
     * Removes the given socket from the data structure.
     */
    SCTPSocket *removeSocket(SCTPSocket *socket);

    /**
     * Returns the number of sockets stored.
     */
    unsigned int size() { return socketMap.size(); }

    /**
     * Deletes the socket objects.
     */
    void deleteSockets();
};

} // namespace inet

#endif // ifndef __INET_SCTPSOCKETMAP_H
