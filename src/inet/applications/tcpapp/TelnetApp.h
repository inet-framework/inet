//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TELNETAPP_H
#define __INET_TELNETAPP_H

#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

/**
 * An example Telnet client application. The server app should be TcpGenericServerApp.
 */
class INET_API TelnetApp : public TcpAppBase
{
  protected:
    cMessage *timeoutMsg = nullptr;
    int numLinesToType = 0; // lines (commands) the user will type in this session
    int numCharsToType = 0; // characters the user will type for current line (command)
    simtime_t stopTime;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleTimer(cMessage *msg) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void checkedScheduleAt(simtime_t t, cMessage *msg);
    virtual void sendGenericAppMsg(int numBytes, int expectedReplyBytes);

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    TelnetApp() {}
    virtual ~TelnetApp();
};

} // namespace inet

#endif

