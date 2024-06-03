//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LOOPBACK_H
#define __INET_LOOPBACK_H

#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace inet::queueing;

class NetworkInterface;

/**
 * Loopback interface implementation.
 */
class INET_API Loopback : public MacProtocolBase, public IPassivePacketSink
{
  protected:
    // statistics
    long numSent = 0;
    long numRcvdOK = 0;

  protected:
    virtual void configureNetworkInterface() override;

  public:
    Loopback() {}
    virtual ~Loopback();

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("appIn") || gate->isName("ipIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("appIn") || gate->isName("ipIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void refreshDisplay() const override;
};

} // namespace inet

#endif

