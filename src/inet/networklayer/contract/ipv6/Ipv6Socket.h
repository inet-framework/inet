//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6SOCKET_H
#define __INET_IPV6SOCKET_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/INetworkSocket.h"

namespace inet {

/**
 * This class implements a raw IPv6 socket.
 */
class INET_API Ipv6Socket : public INetworkSocket
{
  public:
    class INET_API ICallback : public INetworkSocket::ICallback {
      public:
        virtual void socketDataArrived(INetworkSocket *socket, Packet *packet) override { socketDataArrived(check_and_cast<Ipv6Socket *>(socket), packet); }
        virtual void socketDataArrived(Ipv6Socket *socket, Packet *packet) = 0;

        /**
         * Notifies about socket closed, indication ownership is transferred to the callee.
         */
        virtual void socketClosed(INetworkSocket *socket) override { socketClosed(check_and_cast<Ipv6Socket *>(socket)); }
        virtual void socketClosed(Ipv6Socket *socket) = 0;
    };

  protected:
    bool bound = false;
    bool isOpen_ = false;
    int socketId = -1;
    INetworkSocket::ICallback *callback = nullptr;
    void *userData = nullptr;
    cGate *outputGate = nullptr;

  protected:
    void sendToOutput(cMessage *message);

  public:
    Ipv6Socket(cGate *outputGate = nullptr);
    virtual ~Ipv6Socket() {}

    /**
     * Sets the gate on which to send raw packets. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("ipOut"));</tt>
     */
    void setOutputGate(cGate *outputGate) { this->outputGate = outputGate; }
    virtual void setCallback(INetworkSocket::ICallback *callback) override;

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }

    virtual int getSocketId() const override { return socketId; }
    virtual const Protocol *getNetworkProtocol() const override { return &Protocol::ipv6; }

    virtual bool belongsToSocket(cMessage *msg) const override;
    virtual void processMessage(cMessage *msg) override;

    virtual void bind(const Protocol *protocol, Ipv6Address localAddress);
    virtual void connect(Ipv6Address remoteAddress);
    virtual void send(Packet *packet) override;
    virtual void sendTo(Packet *packet, Ipv6Address destAddress);
    virtual void close() override;
    virtual void destroy() override;
    virtual bool isOpen() const override { return isOpen_; }

  protected:
    virtual void bind(const Protocol *protocol, L3Address localAddress) override { bind(protocol, localAddress.toIpv6()); }
    virtual void connect(L3Address remoteAddress) override { connect(remoteAddress.toIpv6()); }
    virtual void sendTo(Packet *packet, L3Address destAddress) override { sendTo(packet, destAddress.toIpv6()); }
};

} // namespace inet

#endif

