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
 * ~CompoundPacketQueueBase's shared-capacity overflow handling: when the drop-from-longest
 * dropper removes a frame from a per-station sub-queue, the frame is still owned by that
 * sub-queue, so this class takes ownership before the base class deletes it -- otherwise
 * OMNeT++ warns about deleting an object owned by another module.
 *
 * See the corresponding NED file for the submodule structure.
 *
 * @see AirtimeFairnessClassifier, AirtimeFairnessGate, AirtimeFairnessScheduler,
 *      Ieee80211LongestFlowDropper
 */
class INET_API AirtimeFairnessCompoundQueue : public queueing::CompoundPacketQueueBase
{
  public:
    virtual void removePacket(Packet *packet) override;
};

} // namespace ieee80211
} // namespace inet

#endif
