//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPCLIENTSOCKETIO_H
#define __INET_TCPCLIENTSOCKETIO_H

#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

using namespace inet::queueing;

class INET_API TcpClientSocketIo : public cSimpleModule, public TcpSocket::ICallback, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef trafficSink;
    TcpSocket socket;
    cMessage *readDelayTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void open();

  public:
    virtual ~TcpClientSocketIo();

    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override;
    virtual void socketDeleted(TcpSocket *socket) override;

    virtual void sendOrScheduleReadCommandIfNeeded();

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("trafficIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("trafficIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

