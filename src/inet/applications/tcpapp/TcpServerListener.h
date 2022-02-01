//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSERVERLISTENER_H
#define __INET_TCPSERVERLISTENER_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/tcpapp/TcpServerSocketIo.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

class INET_API TcpServerListener : public ApplicationBase, public TcpSocket::ICallback
{
  protected:
    int connectionId = 0;
    TcpSocket serverSocket;
    std::set<TcpServerSocketIo *> connectionSet;
    static const char *submoduleVectorName;

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override { throw cRuntimeError("Unexpected data"); }
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override {}
    virtual void socketPeerClosed(TcpSocket *socket) override {}
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override {}
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override {}

    virtual void removeConnection(TcpServerSocketIo *connection);
    virtual void connectionClosed(TcpServerSocketIo *connection);
};

} // namespace inet

#endif

