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
 * The compound-module class behind the airtime-fairness compound queue. It only publishes the
 * number of per-station branches created so far as a `numStations` watch, so the queue's
 * display string can refer to it as {numStations}.
 *
 * See the corresponding NED file for the submodule structure.
 *
 * @see DynamicClassifier, AirtimeFairnessGate, AirtimeFairnessScheduler,
 *      Ieee80211LongestFlowDropper
 */
class INET_API AirtimeFairnessCompoundQueue : public queueing::CompoundPacketQueueBase
{
  protected:
    virtual void initialize(int stage) override;
};

} // namespace ieee80211
} // namespace inet

#endif
