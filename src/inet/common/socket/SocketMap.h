//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SOCKETMAP_H
#define __INET_SOCKETMAP_H

#include <map>

#include "inet/common/socket/ISocket.h"

namespace inet {

/**
 * Small utility class for managing a large number of ISocket objects.
 */
class INET_API SocketMap
{
  protected:
    std::map<int, ISocket *> socketMap; // socketId, socketPtr

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

#endif

