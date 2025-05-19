//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_QUIC_QUICDISCARDSERVER_H_
#define __INET_QUIC_QUICDISCARDSERVER_H_

#include <omnetpp.h>
#include "inet/transportlayer/contract/quic/QuicSocket.h"
#include "inet/applications/base/ApplicationBase.h"

using namespace omnetpp;

namespace inet {

class QuicDiscardServer : public ApplicationBase, public QuicSocket::ICallback
{
  public:
    ~QuicDiscardServer();

  protected:
    QuicSocket listeningSocket;
    std::vector<QuicSocket *> clientSockets;

  protected:
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(QuicSocket* socket, Packet *packet) override;
    virtual void socketConnectionAvailable(QuicSocket *socket) override;
    virtual void socketDataAvailable(QuicSocket* socket, QuicDataInfo *dataInfo) override;
    virtual void socketEstablished(QuicSocket *socket) override;
    virtual void socketClosed(QuicSocket *socket) override;
    virtual void socketDestroyed(QuicSocket *socket) override;

    virtual void socketSendQueueFull(QuicSocket *socket) override { };
    virtual void socketSendQueueDrain(QuicSocket *socket) override { };
    virtual void socketMsgRejected(QuicSocket *socket) override { };
};

} //namespace

#endif
