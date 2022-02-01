//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSINKAPP_H
#define __INET_TCPSINKAPP_H

#include "inet/applications/tcpapp/TcpServerHostApp.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

/**
 * Accepts any number of incoming connections, and discards whatever arrives
 * on them.
 */
class INET_API TcpSinkApp : public TcpServerHostApp
{
  protected:
    long bytesRcvd = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;
    virtual void refreshDisplay() const override;

  public:
    TcpSinkApp();
    ~TcpSinkApp();

    friend class TcpSinkAppThread;
};

class INET_API TcpSinkAppThread : public TcpServerThreadBase
{
  protected:
    long bytesRcvd;
    TcpSinkApp *sinkAppModule = nullptr;

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void refreshDisplay() const override;

    // TcpServerThreadBase:
    /**
     * Called when connection is established.
     */
    virtual void established() override;

    /*
     * Called when a data packet arrives.
     */
    virtual void dataArrived(Packet *msg, bool urgent) override;

    /*
     * Called when a timer (scheduled via scheduleAt()) expires.
     */
    virtual void timerExpired(cMessage *timer) override { throw cRuntimeError("Model error: unknown timer message arrived"); }

    virtual void init(TcpServerHostApp *hostmodule, TcpSocket *socket) override { TcpServerThreadBase::init(hostmodule, socket); sinkAppModule = check_and_cast<TcpSinkApp *>(hostmod); }
};

} // namespace inet

#endif

