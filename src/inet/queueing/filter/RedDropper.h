//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_REDDROPPER_H
#define __INET_REDDROPPER_H

#include "inet/common/ModuleRef.h"
#include "inet/common/packet/Packet.h"
#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace queueing {

/**
 * Implementation of Random Early Detection (RED).
 */
class INET_API RedDropper : public PacketFilterBase, public cListener
{
  protected:
    enum RedResult { QUEUE_FULL, RANDOMLY_ABOVE_LIMIT, RANDOMLY_BELOW_LIMIT, ABOVE_MAX_LIMIT, BELOW_MIN_LIMIT };

    // the result of the RED algorithm for a packet, and the state after it
    struct RedDecision {
        RedResult result = BELOW_MIN_LIMIT;
        double avg = 0.0;
        double count = NaN;
        simtime_t q_time;
        bool usesRandomValue = false;
    };

  protected:
    double wq = 0.0;
    double minth = NaN;
    double maxth = NaN;
    double maxp = NaN;
    double pkrate = NaN;
    double count = NaN;

    double avg = 0.0;
    simtime_t q_time;

    int packetCapacity = -1;
    bool useEcn = false;
    bool markNext = false;
    double randomValue = NaN; // drawn before the decision that uses it, so that a query draws nothing

    ModuleRef<IPacketCollection> collection;

  protected:
    virtual void initialize(int stage) override;

    virtual RedDecision doRandomEarlyDetection(const Packet *packet) const;
    virtual bool matchesResult(RedResult result, const Packet *packet) const;

    virtual void handlePacketFiltered(Packet *packet, bool matches) override;

    // cListener:
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace queueing
} // namespace inet

#endif

