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

#ifndef __INET_SOCKETBASE_H
#define __INET_SOCKETBASE_H

#include "inet/common/socket/ISocket.h"

namespace inet {

class INET_API SocketBase : public ISocket
{
  protected:
    cGate *outputGate = nullptr;
    int socketId = -1;
    bool isOpen_ = false;
    void *userData = nullptr;

  protected:
    virtual void sendOut(cMessage *msg);

  public:
    SocketBase();
    virtual ~SocketBase() { }

    /** @name Setting up a socket */
    //@{
    /**
     * Sets the gate on which to send messages. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("out"));</tt>
     */
    void setOutputGate(cGate *gate) { this->outputGate = gate; }

    /**
     * Returns the internal socket Id.
     */
    int getSocketId() const override { return socketId; }
    //@}

    /** @name Opening and closing connections, sending data */
    //@{
    /**
     * Sends a data packet to the address and port specified previously
     * in a connect() call.
     */
    virtual void send(Packet *packet) override;

    /**
     * Returns true if the socket is open.
     */
    virtual bool isOpen() const override { return isOpen_; }

    /**
     * Unbinds the socket. Once closed, a closed socket may be bound to another
     * (or the same) port, and reused.
     */
    virtual void close() override;

    /**
     * Notify the protocol that the owner of ISocket has destroyed the socket.
     * Typically used when the owner of ISocket has crashed.
     */
    virtual void destroy() override;
    //@}

    /** @name Handling of messages */
    //@{
    /**
     * Returns true if the message belongs to this socket instance.
     */
    virtual bool belongsToSocket(cMessage *msg) const override;
    //@}

    /** @name User data */
    //@{
    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }
    //@}
};

} // namespace inet

#endif

