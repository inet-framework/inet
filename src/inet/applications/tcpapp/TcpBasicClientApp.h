//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPBASICCLIENTAPP_H
#define __INET_TCPBASICCLIENTAPP_H

#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

/**
 * An example request-reply based client application.
 */
class INET_API TcpBasicClientApp : public TcpAppBase
{
  protected:
    cMessage *timeoutMsg = nullptr;
    bool earlySend = false; // if true, don't wait with sendRequest() until established()
    int numRequestsToSend = 0; // requests to send in this session
    simtime_t startTime;
    simtime_t stopTime;

    virtual void sendRequest();
    virtual void rescheduleAfterOrDeleteTimer(simtime_t d, short int msgKind);

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleTimer(cMessage *msg) override;

    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void close() override;

  public:
    TcpBasicClientApp() {}
    virtual ~TcpBasicClientApp();
};

} // namespace inet

#endif

