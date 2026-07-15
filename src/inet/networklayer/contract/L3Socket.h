//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_L3SOCKET_H
#define __INET_L3SOCKET_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/INetworkSocket.h"
#include "inet/networklayer/contract/IL3Protocol.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

/**
 * This class implements a raw L3 socket.
 */
class INET_API L3Socket : public INetworkSocket, public IL3Protocol::ICallback
{
  public:
    class INET_API ICallback : public INetworkSocket::ICallback {
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
    PassivePacketSinkRef sink;
    ModuleRefByGate<IL3Protocol> l3ProtocolModule;

  protected:
    void sendToOutput(cMessage *message);

  public:
    L3Socket(const Protocol *l3Protocol, cGate *outputGate = nullptr);
    virtual ~L3Socket() {}

    /**
     * Sets the gate on which to send raw packets. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("ipOut"));</tt>
     */
    void setOutputGate(cGate *outputGate) {
        this->outputGate = outputGate;
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(l3Protocol);
        dispatchProtocolReq.setServicePrimitive(SP_REQUEST);
        sink.reference(outputGate, true, &dispatchProtocolReq);
        l3ProtocolModule.reference(outputGate, true);
    }
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

    virtual void handleClosed() override {
        if (callback != nullptr)
            callback->socketClosed(this);
    }
};

} // namespace inet

#endif

