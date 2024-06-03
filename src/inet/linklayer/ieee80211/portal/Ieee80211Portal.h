//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211PORTAL_H
#define __INET_IEEE80211PORTAL_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

namespace ieee80211 {

using namespace inet::queueing;

class INET_API Ieee80211Portal : public cSimpleModule, public IIeee80211Llc, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef lowerLayerSink;
    PassivePacketSinkRef upperLayerSink;

    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    bool upperLayerOutConnected = false;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void encapsulate(Packet *packet);
    virtual void decapsulate(Packet *packet);

  public:
    const Protocol *getProtocol() const override { return &Protocol::ieee8022llc; }

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("trafficIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("trafficIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace ieee80211

} // namespace inet

#endif

