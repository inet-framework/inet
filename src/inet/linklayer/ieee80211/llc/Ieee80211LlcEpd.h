//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211LLCEPD_H
#define __INET_IEEE80211LLCEPD_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {
namespace ieee80211 {

using namespace inet::queueing;

class INET_API Ieee80211LlcEpd : public cSimpleModule, public IIeee80211Llc, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef lowerLayerSink;
    PassivePacketSinkRef upperLayerSink;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void encapsulate(Packet *frame);
    virtual void decapsulate(Packet *frame);

  public:
    const Protocol *getProtocol() const override;

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("upperLayerIn") || gate->isName("lowerLayerIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("upperLayerIn") || gate->isName("lowerLayerIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace ieee80211
} // namespace inet

#endif

