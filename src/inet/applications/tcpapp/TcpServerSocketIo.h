//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSERVERSOCKETIO_H
#define __INET_TCPSERVERSOCKETIO_H

#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

class INET_API TcpServerSocketIo : public cSimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpSocket *socket = nullptr;

  protected:
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual ~TcpServerSocketIo() { delete socket; }

    virtual TcpSocket *getSocket() { return socket; }
    virtual void acceptSocket(TcpAvailableInfo *availableInfo);

    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override {}
    virtual void socketEstablished(TcpSocket *socket) override {}
    virtual void socketPeerClosed(TcpSocket *socket) override {}
    virtual void socketClosed(TcpSocket *socket) override {}
    virtual void socketFailure(TcpSocket *socket, int code) override {}
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override {}
};

} // namespace inet

#endif

