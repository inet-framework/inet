//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TUNSOCKET_H
#define __INET_TUNSOCKET_H

#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"

namespace inet {

class INET_API TunSocket : public ISocket
{
  public:
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}
        virtual void socketDataArrived(TunSocket *socket, Packet *packet) = 0;
        virtual void socketClosed(TunSocket *socket) = 0;
    };

  protected:
    int socketId = -1;
    int interfaceId = -1;
    ICallback *callback = nullptr;
    void *userData = nullptr;
    cGate *outputGate = nullptr;
    bool isOpen_ = false;

  protected:
    void sendToTun(cMessage *msg);

  public:
    TunSocket();
    ~TunSocket() {}

    /**
     * Sets the gate on which to send raw packets. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("ipOut"));</tt>
     */
    void setOutputGate(cGate *outputGate) { this->outputGate = outputGate; }

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public TunSocket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * TunSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallback(ICallback *cb);

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }

    /**
     * Returns the internal socket Id.
     */
    virtual int getSocketId() const override { return socketId; }

    void open(int interfaceId);
    virtual void send(Packet *packet) override;
    virtual void close() override;
    virtual void destroy() override;
    virtual bool isOpen() const override { return isOpen_; }

    virtual bool belongsToSocket(cMessage *msg) const override;
    virtual void processMessage(cMessage *msg) override;
};

} // namespace inet

#endif

