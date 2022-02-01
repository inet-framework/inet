//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#ifndef __INET_HTTPSERVER_H
#define __INET_HTTPSERVER_H

#include "inet/applications/httptools/server/HttpServerBase.h"
#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

namespace httptools {

/**
 * HttpServerDirect module.
 *
 * This module implements a flexible Web server. It is part of the HttpTools project
 * and should be used in conjunction with a number of clients running the HttpBrowserDirect.
 *
 * @see HttpBrowserDirect
 *
 * @author  Kristjan V. Jonsson
 */
class INET_API HttpServer : public HttpServerBase, public TcpSocket::ReceiveQueueBasedCallback
{
  protected:
    struct SockData {
        TcpSocket *socket = nullptr; // A reference to the socket object.
    };

    TcpSocket listensocket;
    SocketMap sockCollection;
    unsigned long numBroken = 0;
    unsigned long socketsOpened = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void socketDataArrived(TcpSocket *socket) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override { socket->accept(availableInfo->getNewSocketId()); }
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override;
};

} // namespace httptools

} // namespace inet

#endif

