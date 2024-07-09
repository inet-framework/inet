//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TUNLOOPBACKAPP_H
#define __INET_TUNLOOPBACKAPP_H

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/linklayer/tun/TunSocket.h"

namespace inet {

using namespace inet::queueing;

class INET_API TunLoopbackApp : public cSimpleModule, public LifecycleUnsupported, public IPassivePacketSink, public IModuleInterfaceLookup
{
  protected:
    const char *tunInterface = nullptr;

    unsigned int packetsSent = 0;
    unsigned int packetsReceived = 0;

    TunSocket tunSocket;

  protected:
    void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void handleMessage(cMessage *msg) override;
    void finish() override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;
};

} // namespace inet

# endif

