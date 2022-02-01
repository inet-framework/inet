//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TUNNELAPP_H
#define __INET_TUNNELAPP_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/linklayer/tun/TunSocket.h"
#include "inet/networklayer/contract/ipv4/Ipv4Socket.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

class INET_API TunnelApp : public ApplicationBase, public UdpSocket::ICallback, public Ipv4Socket::ICallback, public TunSocket::ICallback
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

