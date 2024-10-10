//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSERVERSOCKETIO_H
#define __INET_TCPSERVERSOCKETIO_H

#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

using namespace inet::queueing;

class INET_API TcpServerSocketIo : public cSimpleModule, public TcpSocket::ICallback, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef trafficSink;
    TcpSocket *socket = nullptr;
    cMessage *readDelayTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
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

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("trafficIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("trafficIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

