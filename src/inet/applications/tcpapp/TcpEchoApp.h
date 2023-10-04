//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPECHOAPP_H
#define __INET_TCPECHOAPP_H

#include "inet/applications/tcpapp/ITcpServerSocketIo.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */
class INET_API TcpEchoAppThread : public cSimpleModule, public TcpSocket::ICallback, public ITcpServerSocketIo
{
  protected:
    TcpSocket *socket = nullptr;
    cMessage *readDelayTimer = nullptr;
    int64_t bytesRcvd = 0;
    int64_t bytesSent = 0;
    double echoFactor = NaN;
    std::set<Packet *> delayedPackets;

    virtual void sendDown(Packet *packet);
    virtual void clearDelayedPackets();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *message) override;
    virtual void refreshDisplay() const override;

  public:
    ~TcpEchoAppThread();

    virtual TcpSocket *getSocket() override { return socket; }
    virtual void acceptSocket(TcpAvailableInfo *availableInfo) override;
    virtual void close() override { socket->close(); }
    virtual void deleteModule() override { cSimpleModule::deleteModule(); }

    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override;
    virtual void socketDeleted(TcpSocket *socket) override;

    virtual void sendOrScheduleReadCommandIfNeeded();
};

} // namespace inet

#endif

