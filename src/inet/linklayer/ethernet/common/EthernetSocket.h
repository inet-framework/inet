//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETSOCKET_H
#define __INET_ETHERNETSOCKET_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketBase.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class INET_API EthernetSocket : public SocketBase
{
  public:
    class INET_API ICallback {
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
    ICallback *callback = nullptr;
    NetworkInterface *networkInterface = nullptr;

  protected:
    virtual void sendOut(cMessage *msg) override;
    virtual void sendOut(Request *request) override;
    virtual void sendOut(Packet *packet) override;

  public:
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
    void setCallback(ICallback *callback) { this->callback = callback; }

    void setNetworkInterface(NetworkInterface *networkInterface) { this->networkInterface = networkInterface; }

    /**
     * Binds the socket to the MAC address.
     */
    void bind(const MacAddress& localAddress, const MacAddress& remoteAddress, const Protocol *protocol, bool steal);
    //@}

    /** @name Handling of messages arriving from Ethernet */
    //@{
    virtual void processMessage(cMessage *msg) override;
    //@}
};

} // namespace inet

#endif

