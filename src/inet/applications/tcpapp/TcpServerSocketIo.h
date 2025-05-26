//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSERVERSOCKETIO_H
#define __INET_TCPSERVERSOCKETIO_H

#include "inet/common/SimpleModule.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

class INET_API TcpServerSocketIo : public SimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpSocket *socket = nullptr;
    cMessage *readDelayTimer = nullptr;

  protected:
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual ~TcpServerSocketIo() { cancelAndDelete(readDelayTimer); delete socket; }

    virtual TcpSocket *getSocket() { return socket; }
    virtual void acceptSocket(TcpAvailableInfo *availableInfo);
    virtual void close() { socket->close(); }

    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override {}
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override {}
    virtual void socketClosed(TcpSocket *socket) override { if (readDelayTimer) cancelEvent(readDelayTimer); }
    virtual void socketFailure(TcpSocket *socket, int code) override { if (readDelayTimer) cancelEvent(readDelayTimer); }
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override { ASSERT(socket == this->socket); if (readDelayTimer) cancelEvent(readDelayTimer); socket = nullptr; }

    virtual void sendOrScheduleReadCommandIfNeeded();
};

} // namespace inet

#endif

