//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VIRTUALTUNNEL_H
#define __INET_VIRTUALTUNNEL_H

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "inet/queueing/base/PassivePacketSinkBase.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/EthernetSocket.h"
#endif

#ifdef INET_WITH_IEEE8021Q
#include "inet/linklayer/ieee8021q/Ieee8021qSocket.h"
#endif

#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class INET_API VirtualTunnel : public queueing::PassivePacketSinkBase
#ifdef INET_WITH_ETHERNET
        , public EthernetSocket::ICallback
#endif
#ifdef INET_WITH_IEEE8021Q
        , public Ieee8021qSocket::ICallback
#endif
{
  protected:
    NetworkInterface *realNetworkInterface = nullptr;
    NetworkInterface *networkInterface = nullptr;
    const Protocol *protocol = nullptr;
    int vlanId = -1;

    ISocket *socket = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;

#ifdef INET_WITH_ETHERNET
    virtual void socketDataArrived(EthernetSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(EthernetSocket *socket, Indication *indication) override { throw cRuntimeError("Invalid operation"); }
    virtual void socketClosed(EthernetSocket *socket) override {}
#endif

#ifdef INET_WITH_IEEE8021Q
    virtual void socketDataArrived(Ieee8021qSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(Ieee8021qSocket *socket, Indication *indication) override { throw cRuntimeError("Invalid operation"); }
    virtual void socketClosed(Ieee8021qSocket *socket) override {}
#endif

  public:
    virtual ~VirtualTunnel() { delete socket; }
};

} // namespace inet

#endif

