//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AIRTIMEFAIRNESSCOMPOUNDQUEUE_H
#define __INET_AIRTIMEFAIRNESSCOMPOUNDQUEUE_H

#include "inet/queueing/queue/CompoundPacketQueueBase.h"

namespace inet {
namespace ieee80211 {

/**
 * The compound-module class behind the airtime-fairness compound queue. It only refines
 * ~CompoundPacketQueueBase's shared-capacity overflow handling. The base class, on overflow,
 * removes the victim frame from its per-station sub-queue by calling removePacket() (which
 * emits packetRemoved) and then dropPacket() (which emits packetDropped); that has two
 * problems for a sub-queue-based compound: the frame stays owned by the sub-queue so the
 * subsequent delete warns about deleting an object it does not own, and the frame is counted
 * as both removed and dropped, so it is subtracted twice from the queue-length statistic
 * (which then drifts). This class overrides pushPacket() to remove the victim directly from
 * the sub-queue collection (no spurious packetRemoved), take ownership before the delete, and
 * drop it exactly once.
 *
 * See the corresponding NED file for the submodule structure.
 *
 * @see AirtimeFairnessClassifier, AirtimeFairnessGate, AirtimeFairnessScheduler,
 *      Ieee80211LongestFlowDropper
 */
class INET_API AirtimeFairnessCompoundQueue : public queueing::CompoundPacketQueueBase
{
  public:
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
};

} // namespace ieee80211
} // namespace inet

#endif
