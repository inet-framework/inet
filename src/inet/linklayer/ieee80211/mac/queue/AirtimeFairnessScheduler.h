//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AIRTIMEFAIRNESSSCHEDULER_H
#define __INET_AIRTIMEFAIRNESSSCHEDULER_H

#include <vector>

#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace ieee80211 {

class AirtimeFairnessGate;

/**
 * The scheduling half of the ~AirtimeFairnessQueue compound queue; the counterpart of
 * ~AirtimeFairnessGate. It runs a deficit round robin (DRR) over the per-station gates,
 * as in Linux `mac80211` airtime fairness: it visits the gates in round-robin order and
 * serves the first backlogged station that is still eligible (its gate is open, i.e. its
 * airtime deficit is non-negative); an ineligible-but-backlogged station is granted one
 * `quantum * weight` of airtime credit (see AirtimeFairnessGate::addQuantum()) and the
 * cursor moves on. Over time every backlogged station gets an equal share of on-air time.
 *
 * The airtime deficit itself lives in the gates, charged a-posteriori from the actual
 * on-air duration of each transmitted frame; this scheduler only owns the rotation and
 * the top-up trigger, so the two form a matched pair. Because a closed gate hides its
 * backlog from the generic pull interface, the scheduler consults the gates through their
 * typed interface (isBacklogged()/getDeficit()/peekPacket()) rather than the plain
 * provider references, and reports a pullable packet whenever *any* station is backlogged
 * (even if every gate is momentarily closed) since it can always top a station up.
 *
 * Like the other schedulers used inside a compound queue, it also implements
 * ~IPacketCollection, aggregating the per-station backlog so the enclosing
 * ~CompoundPacketQueueBase can report queue length and enforce the shared capacity.
 *
 * With `fairnessEnabled = false` the gates stay open and this degrades to a plain
 * per-station round robin (the frame-fair anomaly baseline).
 *
 * See the corresponding NED file for more details.
 *
 * @see AirtimeFairnessQueue, AirtimeFairnessGate
 */
class INET_API AirtimeFairnessScheduler : public queueing::PacketSchedulerBase, public virtual queueing::IPacketCollection
{
  protected:
    // parameters
    bool fairnessEnabled = true; // when false, degrades to a plain per-station round robin

    // state
    int cursor = 0; // round-robin position: the input gate index considered first
    std::vector<AirtimeFairnessGate *> gates; // the gate feeding each input gate, parallel to inputGates

  protected:
    virtual void initialize(int stage) override;
    virtual int schedulePacket() override;
    virtual void resolveGates();

    virtual AirtimeFairnessGate *getGate(int index) const { return gates[index]; }

  public:
    /**
     * Registers an input gate added at runtime (a per-station branch created on demand by
     * ~AirtimeFairnessClassifier): appends the provider/producer references and the matched
     * gate, and notifies the downstream collector that a new station may be pullable.
     */
    virtual void addInput(cGate *inputGate);

    /** @name Pull interface */
    //@{
    virtual bool canPullSomePacket(const cGate *gate) const override;
    virtual Packet *canPullPacket(const cGate *gate) const override;
    //@}

    /** @name IPacketCollection: aggregate per-station backlog for the compound queue */
    //@{
    virtual int getMaxNumPackets() const override { return -1; }
    virtual int getNumPackets() const override;

    virtual b getMaxTotalLength() const override { return b(-1); }
    virtual b getTotalLength() const override;

    virtual bool isEmpty() const override { return getNumPackets() == 0; }
    virtual Packet *getPacket(int index) const override;
    virtual void removePacket(Packet *packet) override;
    virtual void removeAllPackets() override;
    //@}
};

} // namespace ieee80211
} // namespace inet

#endif
