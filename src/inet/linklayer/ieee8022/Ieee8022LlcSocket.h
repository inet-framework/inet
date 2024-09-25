//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022LLCSOCKET_H
#define __INET_IEEE8022LLCSOCKET_H

#include "inet/common/socket/SocketBase.h"
#include "inet/linklayer/ieee8022/IIeee8022Llc.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8022LlcSocket : public SocketBase, public IIeee8022Llc::ICallback
{
  public:
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}
        virtual void socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet) = 0;
        virtual void socketClosed(Ieee8022LlcSocket *socket) = 0;
    };

  protected:
    PassivePacketSinkRef sink;
    ModuleRefByGate<IIeee8022Llc> llc;

    int interfaceId = -1;
    int localSap = -1;
    int remoteSap = -1;
    ICallback *callback = nullptr;

  protected:
    virtual void sendOut(cMessage *msg) override;

  public:
    virtual void setOutputGate(cGate *gate) override {
        SocketBase::setOutputGate(gate);
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(&Protocol::ieee8022llc);
        dispatchProtocolReq.setServicePrimitive(SP_REQUEST);
        sink.reference(gate, true, &dispatchProtocolReq);
        llc.reference(gate, true);
    }

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public LlcSocket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * LlcSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallback(ICallback *callback) { this->callback = callback; }

    /**
     * Returns the interface Id.
     */
    virtual int getInterfaceId() const { return interfaceId; }

    /**
     * Returns the local SAP.
     */
    virtual int getLocalSap() const { return localSap; }

    /**
     * Returns the remote SAP.
     */
    virtual int getRemoteSap() const { return remoteSap; }

    virtual void open(int interfaceId, int localSap, int remoteSap);
    virtual void close() override;
    virtual void processMessage(cMessage *msg) override;

    virtual void send(Packet *msg) override;

    virtual void handleClosed() override {
        if (callback)
            callback->socketClosed(this);
    }
};

} // namespace inet

#endif

