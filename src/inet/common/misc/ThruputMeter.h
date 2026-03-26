//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_THRUPUTMETER_H
#define __INET_THRUPUTMETER_H

#include "inet/common/INETDefs.h"
#include "inet/common/SimpleModule.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

/**
 * Measures and records network thruput
 */
// FIXME problem: if traffic suddenly stops, it'll show the last reading forever;
// (output vector will be correct though); would need a timer to handle this situation
class INET_API ThruputMeter : public SimpleModule, public IPassivePacketSink
{
  protected:
    // config
    simtime_t startTime; // start time
    unsigned int batchSize; // number of packets in a batch
    simtime_t maxInterval; // max length of measurement interval (measurement ends
    // if either batchSize or maxInterval is reached, whichever
    // is reached first)

    // global statistics
    unsigned long numPackets;
    unsigned long numBits;

    // current measurement interval
    simtime_t intvlStartTime;
    simtime_t intvlLastPkTime;
    unsigned long intvlNumPackets;
    unsigned long intvlNumBits;

    // statistics
    cOutVector bitpersecVector;
    cOutVector pkpersecVector;

    PassivePacketSinkRef outSink;

  protected:
    virtual void updateStats(simtime_t now, unsigned long bits);
    virtual void beginNewInterval(simtime_t now);

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

