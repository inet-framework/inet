//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISOCKET_H
#define __INET_ISOCKET_H

#include "inet/common/packet/Packet.h"

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

    virtual void send(Packet *packet) = 0;

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

#endif

