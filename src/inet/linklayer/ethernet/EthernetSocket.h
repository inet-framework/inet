//
// Copyright (C) OpenSim Ltd.
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_ETHERNETSOCKET_H
#define __INET_ETHERNETSOCKET_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

class INET_API EthernetSocket : public ISocket
{
  public:
    class INET_API ICallback
    {
      public:
        virtual ~ICallback() {}

        /**
         * Notifies about data arrival, packet ownership is transferred to the callee.
         */
        virtual void socketDataArrived(EthernetSocket *socket, Packet *packet) = 0;

        /**
         * Notifies about error indication arrival, indication ownership is transferred to the callee.
         */
        virtual void socketErrorArrived(EthernetSocket *socket, Indication *indication) = 0;

        /**
         * Notifies about the socket closed.
         */
        virtual void socketClosed(EthernetSocket *socket) = 0;
    };
  protected:
    int socketId;
    ICallback *callback = nullptr;
    void *userData = nullptr;
    InterfaceEntry *interfaceEntry = nullptr;
    cGate *gateToEthernet = nullptr;
    bool isOpen_ = false;

  protected:
    void sendToEthernet(cMessage *msg);

  public:
    EthernetSocket();
    virtual ~EthernetSocket() {}

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }

    /**
     * Returns the internal socket Id.
     */
    int getSocketId() const override { return socketId; }

    /** @name Opening and closing connections, sending data */
    //@{
    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public EthernetSocket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * EthernetSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallback(ICallback *cb);

    /**
     * Sets the gate on which to send to Ethernet. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("out"));</tt>
     */
    void setOutputGate(cGate *gate) { gateToEthernet = gate; }

    void setInterfaceEntry(InterfaceEntry *interfaceEntry) { this->interfaceEntry = interfaceEntry; }

    /**
     * Binds the socket to the MAC address.
     */
    void bind(const MacAddress& sourceAddress, const MacAddress& destinationAddress, Protocol *protocol, int vlanId);

    /**
     * Sends a data packet to the address and port specified previously
     * in a connect() call.
     */
    void send(Packet *packet);

    virtual bool isOpen() const override { return isOpen_; }
    /**
     * Unbinds the socket. Once closed, a closed socket may be bound to another
     * (or the same) port, and reused.
     */
    virtual void close() override;
    //@}

    /**
     * Notify the protocol that the owner of ISocket has destroyed the socket.
     * Typically used when the owner of ISocket has crashed.
     */
    virtual void destroy() override;

    /** @name Handling of messages arriving from Ethernet */
    //@{
    /**
     * Returns true if the message belongs to this socket instance.
     */
    virtual bool belongsToSocket(cMessage *msg) const override;
    virtual void processMessage(cMessage *msg) override;
    //@}
};

} // namespace inet

#endif // ifndef __INET_ETHERNETSOCKET_H

