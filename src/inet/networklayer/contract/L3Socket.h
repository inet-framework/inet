//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_L3SOCKET_H
#define __INET_L3SOCKET_H

#include "inet/common/INETDefs.h"
#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"

namespace inet {

/**
 * Thie class implements a raw L3 socket.
 */
class INET_API L3Socket : public ISocket
{
  public:
    class ICallback {
        public:
          virtual ~ICallback() {}
          virtual void socketDataArrived(L3Socket *socket, Packet *packet) = 0;
      };
  protected:
    bool bound = false;
    const Protocol *l3Protocol = nullptr;
    int socketId = -1;
    ICallback *cb = nullptr;
    void *userData = nullptr;
    cGate *outputGate = nullptr;

  protected:
    void sendToOutput(cMessage *message);

  public:
    L3Socket(const Protocol *l3ProtocolId = nullptr, cGate *outputGate = nullptr);
    virtual ~L3Socket() {}

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }

    /**
     * Sets controlInfoProtocolId, for example setControlInfoProtocolId(Protocol::ipv4.getId())
     */
    void setL3Protocol(const Protocol *l3Protocol);

    const Protocol *getL3Protocol() const { return l3Protocol; }


    /**
     * Returns the internal socket Id.
     */
    int getSocketId() const override { return socketId; }

    virtual bool belongsToSocket(cMessage *msg) const override;

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public L3Socket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * L3Socket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallbackObject(ICallback *cb, void *userData = nullptr);

    virtual void processMessage(cMessage *msg) override;


    /**
     * Sets the gate on which to send raw packets. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("ipOut"));</tt>
     */
    void setOutputGate(cGate *outputGate) { this->outputGate = outputGate; }

    /**
     * Bind the socket to a protocol.
     */
    void bind(const Protocol *protocol);

    /**
     * Sends a data packet.
     */
    void send(cPacket *msg);

    /**
     * Unbinds the socket. Once closed, a closed socket may be bound to another
     * (or the same) protocol, and reused.
     */
    void close();
};

} // namespace inet

#endif // ifndef __INET_L3SOCKET_H

