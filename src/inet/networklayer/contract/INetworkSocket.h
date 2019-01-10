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

#ifndef __INET_INETWORKSOCKET_H
#define __INET_INETWORKSOCKET_H

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

/**
 * This class provides an interface that should be implemented by all network sockets.
 */
class INET_API INetworkSocket : public ISocket
{
  public:
    class INET_API ICallback
    {
      public:
        virtual ~ICallback() {}
        virtual void socketDataArrived(INetworkSocket *socket, Packet *packet) = 0;
        virtual void socketClosed(INetworkSocket *socket) = 0;
    };

  public:
    /**
     * Sets a callback object, to be used with processMessage(). This callback
     * object may be your simple module itself (if it multiply inherits from
     * ICallback too, that is you declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use dedicated
     * class (and objects) for this purpose.
     *
     * Sockets don't delete the callback object in the destructor or on any
     * other occasion.
     */
    virtual void setCallback(ICallback *callback) = 0;

    /**
     * Returns the associated network protocol used to deliver datagrams by this socket.
     */
    virtual const Protocol *getNetworkProtocol() const = 0;

    /**
     * Binds this socket to the given protocol and local address. All incoming
     * packets matching the given parameters will be delivered via the callback
     * interface.
     */
    virtual void bind(const Protocol *protocol, L3Address localAddress) = 0;

    /**
     * Connects to a remote socket. The socket will only receive packets from
     * the specified address, and you can use send() as opposed to sendTo() to
     * send packets.
     */
    virtual void connect(L3Address remoteAddress) = 0;

    /**
     * Sends a packet using the associated network protocol.
     */
    virtual void send(Packet *packet) = 0;

    /**
     * Sends a packet to the given remote address using the associated network protocol.
     */
    virtual void sendTo(Packet *packet, L3Address remoteAddress) = 0;

    /**
     * Closes this socket releasing all resources. Once closed, a closed socket
     * may be bound to another (or the same) protocol, and reused.
     */
    virtual void close() = 0;
};

} // namespace inet

#endif // ifndef __INET_INETWORKSOCKET_H

