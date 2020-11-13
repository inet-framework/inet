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

#ifndef __INET_IEEE8021QSOCKET_H
#define __INET_IEEE8021QSOCKET_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketBase.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class INET_API Ieee8021qSocket : public SocketBase
{
  public:
    class INET_API ICallback
    {
      public:
        virtual ~ICallback() {}

        /**
         * Notifies about data arrival, packet ownership is transferred to the callee.
         */
        virtual void socketDataArrived(Ieee8021qSocket *socket, Packet *packet) = 0;

        /**
         * Notifies about error indication arrival, indication ownership is transferred to the callee.
         */
        virtual void socketErrorArrived(Ieee8021qSocket *socket, Indication *indication) = 0;

        /**
         * Notifies about the socket closed.
         */
        virtual void socketClosed(Ieee8021qSocket *socket) = 0;
    };

  protected:
    ICallback *callback = nullptr;
    NetworkInterface *networkInterface = nullptr;
    const Protocol *protocol = nullptr;

  protected:
    virtual void sendOut(cMessage *msg) override;

  public:
    /** @name Opening and closing connections, sending data */
    //@{
    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public Ieee8021qSocket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * Ieee8021qSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallback(ICallback *callback) { this->callback = callback; }
    void setNetworkInterface(NetworkInterface *networkInterface) { this->networkInterface = networkInterface; }
    void setProtocol(const Protocol *protocol) { this->protocol = protocol; }

    /**
     * Binds the socket to the MAC address.
     */
    void bind(const Protocol *protocol, int vlanId, bool steal);
    //@}

    /** @name Handling of messages arriving from Ieee8021q */
    //@{
    /**
     * Returns true if the message belongs to this socket instance.
     */
    virtual void processMessage(cMessage *msg) override;
    //@}
};

} // namespace inet

#endif

