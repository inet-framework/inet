//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPECHOAPP_H
#define __INET_TCPECHOAPP_H

#include "inet/applications/tcpapp/TcpServerHostApp.h"
#include "inet/common/INETMath.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

using namespace inet::queueing;

/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */
class INET_API TcpEchoApp : public TcpServerHostApp
{
  protected:
    PassivePacketSinkRef socketSink;

    simtime_t delay;
    double echoFactor = NaN;

    long bytesRcvd = 0;
    long bytesSent = 0;

  protected:
    virtual void sendDown(Packet *packet);

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;
    virtual void refreshDisplay() const override;

  public:
    TcpEchoApp();
    ~TcpEchoApp();

    friend class TcpEchoAppThread;
};

class INET_API TcpEchoAppThread : public TcpServerThreadBase
{
  protected:
    TcpEchoApp *echoAppModule = nullptr;
    cMessage *readDelayTimer = nullptr;
    Packet *delayedPacket = nullptr;

  public:
    ~TcpEchoAppThread();
    virtual void sendOrScheduleReadCommandIfNeeded();
    virtual void handleMessage(cMessage *msg) override;
    virtual void sendDown(Packet *packet);
    virtual void read();    // send a read request to the socket

    /**
     * Called when connection is established.
     */
    virtual void established() override;

    /*
     * Called when a data packet arrives. To be redefined.
     */
    virtual void dataArrived(Packet *msg, bool urgent) override;

    /*
     * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
     */
    virtual void timerExpired(cMessage *timer) override;

    virtual void init(TcpServerHostApp *hostmodule, TcpSocket *socket) override;

    virtual void close() override;
};

} // namespace inet

#endif

