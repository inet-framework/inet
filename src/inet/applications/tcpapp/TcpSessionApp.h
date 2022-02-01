//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPSESSIONAPP_H
#define __INET_TCPSESSIONAPP_H

#include <vector>

#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

/**
 * Single-connection TCP application.
 */
class INET_API TcpSessionApp : public TcpAppBase
{
  protected:
    // parameters
    struct Command {
        simtime_t tSend;
        long numBytes = 0;
        Command(simtime_t t, long n) { tSend = t; numBytes = n; }
    };
    typedef std::vector<Command> CommandVector;
    CommandVector commands;

    bool activeOpen = false;
    simtime_t tOpen;
    simtime_t tSend;
    simtime_t tClose;
    int sendBytes = 0;

    // state
    int commandIndex = -1;
    cMessage *timeoutMsg = nullptr;

  protected:
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void parseScript(const char *script);
    virtual Packet *createDataPacket(long sendBytes);
    virtual void sendData();

    virtual void handleTimer(cMessage *msg) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;

  public:
    TcpSessionApp() {}
    virtual ~TcpSessionApp();
};

} // namespace inet

#endif

