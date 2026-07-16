//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AIRTIMEFAIRNESSCLASSIFIER_H
#define __INET_AIRTIMEFAIRNESSCLASSIFIER_H

#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {
namespace ieee80211 {

/**
 * The classifying half of the ~AirtimeFairnessQueue compound queue. It routes each frame
 * to a per-receiver sub-queue: the first frame seen for a given receiver (destination
 * station) MAC address is assigned the next free output index, and all later frames for
 * that receiver follow the same branch. This gives one FIFO sub-queue (and one
 * ~AirtimeFairnessGate) per station, as needed by the airtime deficit round robin.
 *
 * This base implementation maps to a fixed set of output gates; the number of stations an
 * access point serves is dynamic, so a subclass creates the sub-queue/gate branch on
 * demand when a never-seen receiver first appears.
 *
 * See the corresponding NED file for more details.
 *
 * @see AirtimeFairnessQueue, AirtimeFairnessGate, AirtimeFairnessScheduler
 */
class INET_API AirtimeFairnessClassifier : public queueing::PacketClassifierBase
{
  protected:
    std::string subqueueModuleType; // NED type of each per-station sub-queue to create on demand
    std::map<MacAddress, int> addressToIndex; // receiver MAC address -> output branch index, assigned in first-seen order

  protected:
    virtual void initialize(int stage) override;
    virtual MacAddress getReceiverAddress(Packet *packet) const;
    virtual int classifyPacket(Packet *packet) override;

    /**
     * Instantiates a new per-station branch: a sub-queue (of type subqueueModuleType) and an
     * ~AirtimeFairnessGate, wired classifier.out[index] -> queue.in, queue.out -> gate.in,
     * gate.out -> scheduler.in[index], and registered with the ~AirtimeFairnessScheduler.
     * Returns the branch index.
     */
    virtual int createBranch(int index);

  public:
    virtual int getNumStations() const { return (int)addressToIndex.size(); }
};

} // namespace ieee80211
} // namespace inet

#endif
