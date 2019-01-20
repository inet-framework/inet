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
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/INetworkSocket.h"

namespace inet {

/**
 * This class implements a raw L3 socket.
 */
class INET_API L3Socket : public INetworkSocket
{
  public:
    class INET_API ICallback : public INetworkSocket::ICallback
    {
      public:
        virtual void socketDataArrived(INetworkSocket *socket, Packet *packet) override { socketDataArrived(check_and_cast<L3Socket *>(socket), packet); }
        virtual void socketDataArrived(L3Socket *socket, Packet *packet) = 0;

        /**
         * Notifies about socket closed, indication ownership is transferred to the callee.
         */
        virtual void socketClosed(INetworkSocket *socket) override { socketClosed(check_and_cast<L3Socket *>(socket)); }
        virtual void socketClosed(L3Socket *socket) = 0;
    };
  protected:
    bool bound = false;
    bool isOpen_ = false;
    const Protocol *l3Protocol = nullptr;
    int socketId = -1;
    INetworkSocket::ICallback *callback = nullptr;
    void *userData = nullptr;
    cGate *outputGate = nullptr;

  protected:
    void sendToOutput(cMessage *message);

  public:
    L3Socket(const Protocol *l3Protocol, cGate *outputGate = nullptr);
    virtual ~L3Socket() {}

    /**
     * Sets the gate on which to send raw packets. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("ipOut"));</tt>
     */
    void setOutputGate(cGate *outputGate) { this->outputGate = outputGate; }
    virtual void setCallback(INetworkSocket::ICallback *callback) override;

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }

    virtual int getSocketId() const override { return socketId; }
    virtual const Protocol *getNetworkProtocol() const override { return l3Protocol; }

    virtual bool belongsToSocket(cMessage *msg) const override;
    virtual void processMessage(cMessage *msg) override;

    virtual void bind(const Protocol *protocol, L3Address localAddress) override;
    virtual void connect(L3Address remoteAddress) override;
    virtual void send(Packet *packet) override;
    virtual void sendTo(Packet *packet, L3Address destAddress) override;
    virtual void close() override;
    virtual void destroy() override;
    virtual bool isOpen() const override { return isOpen_; }
};

} // namespace inet

#endif // ifndef __INET_L3SOCKET_H

