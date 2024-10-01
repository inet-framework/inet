//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TUNNELAPP_H
#define __INET_TUNNELAPP_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/linklayer/tun/TunSocket.h"
#include "inet/networklayer/contract/ipv4/Ipv4Socket.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

using namespace inet::queueing;

class INET_API TunnelApp : public ApplicationBase, public UdpSocket::ICallback, public Ipv4Socket::ICallback, public TunSocket::ICallback, public IPassivePacketSink, public IModuleInterfaceLookup
{
  protected:
    const Protocol *protocol = nullptr;
    const char *interface = nullptr;
    const char *destinationAddress = nullptr;
    int destinationPort = -1;
    int localPort = -1;

    Ipv4Socket ipv4Socket;
    UdpSocket serverSocket;
    UdpSocket clientSocket;
    TunSocket tunSocket;
    SocketMap socketMap;

  public:
    TunnelApp();
    virtual ~TunnelApp();

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // UdpSocket::ICallback
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    // Ipv4Socket::ICallback
    virtual void socketDataArrived(Ipv4Socket *socket, Packet *packet) override;
    virtual void socketClosed(Ipv4Socket *socket) override;

    // TunSocket::ICallback
    virtual void socketDataArrived(TunSocket *socket, Packet *packet) override;
    virtual void socketClosed(TunSocket *socket) override {}

    // OperationalBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override {} // TODO implementation
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif

