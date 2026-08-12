//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AIRTIMEFAIRNESSGATE_H
#define __INET_AIRTIMEFAIRNESSGATE_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/queueing/base/PacketGateBase.h"

namespace inet {
namespace ieee80211 {

/**
 * A per-station airtime-fairness gate for the ~AirtimeFairnessQueue compound queue.
 * One gate sits on the transmit path of exactly one receiver (destination station);
 * it holds that station's airtime `deficit` and is *open* while the station is
 * eligible to transmit (deficit >= 0) and *closed* otherwise. This mirrors the way a
 * ~PeriodicGate gates one sub-queue in a ~GatingPriorityQueue, except that eligibility
 * is driven by on-air time rather than by a schedule.
 *
 * The deficit is charged a-posteriori from the actual on-air duration of each
 * transmitted frame (retries included), reported by the coordination function
 * (~Dcf/~Hcf) via the `frameTransmittedAirtime` signal (see ~Ieee80211AirtimeInd). The
 * gate learns which station it serves from the receiver address of the first frame that
 * passes through it, so that later airtime charges can be matched to it.
 *
 * The gate forms a matched pair with ~AirtimeFairnessScheduler: the scheduler owns the
 * deficit round-robin rotation and tops up an ineligible-but-backlogged gate by one
 * `quantum * weight` when its turn comes (see addQuantum()); the gate owns the deficit
 * state and the open/closed eligibility. Because a closed gate hides its backlog from
 * the generic pull interface, the gate exposes isBacklogged()/peekPacket() for the
 * scheduler and forwards backlog-change notifications even while closed.
 *
 * With `fairnessEnabled = false` the gate stays permanently open, degrading the compound
 * queue to a plain per-station round robin (the frame-fair anomaly baseline).
 *
 * See the corresponding NED file for more details.
 *
 * @see AirtimeFairnessQueue, AirtimeFairnessScheduler, Ieee80211AirtimeInd
 */
class INET_API AirtimeFairnessGate : public queueing::PacketGateBase, public cListener
{
  public:
    static simsignal_t deficitChangedSignal;

  protected:
    // parameters
    simtime_t quantum; // airtime top-up granted per round-robin turn (scaled by weight)
    double weight = 1.0; // per-station airtime weight; equal weight means equal airtime share
    bool fairnessEnabled = true; // when false the gate stays open (plain round robin)

    // state
    MacAddress stationAddress; // receiver this gate serves; learned from the first frame passing through
    simtime_t deficit = SIMTIME_ZERO; // current airtime deficit; the gate is open while this is non-negative

    simsignal_t frameTransmittedAirtimeSignal = SIMSIGNAL_NULL;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

    virtual bool isEligible() const { return !fairnessEnabled || deficit >= SIMTIME_ZERO; }
    virtual void updateGateState();
    virtual void setDeficit(simtime_t value);

  public:
    /** @name Matched-pair interface used by ~AirtimeFairnessScheduler */
    //@{
    /**
     * Returns the station's current airtime deficit; a non-negative value means the
     * station is eligible to transmit.
     */
    virtual simtime_t getDeficit() const { return deficit; }
    /**
     * Grants one round-robin quantum (`quantum * weight`) of airtime credit and reopens
     * the gate if the deficit thereby became non-negative.
     */
    virtual void addQuantum();
    /**
     * Returns whether the upstream sub-queue holds a frame, regardless of whether this
     * gate is currently open or closed.
     */
    virtual bool isBacklogged() const;
    /**
     * Returns the head frame of the upstream sub-queue (regardless of open/closed), or
     * nullptr if the sub-queue is empty.
     */
    virtual Packet *peekPacket() const;
    /**
     * Returns the receiver address this gate serves, or an unspecified address until the
     * first frame has passed through.
     */
    virtual const MacAddress& getStationAddress() const { return stationAddress; }
    //@}

    virtual void handleCanPullPacketChanged(const cGate *gate) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *object, cObject *details) override;

    /** @name IPacketCollection: report the true upstream backlog even while the gate is closed */
    //@{
    // PacketGateBase hides the backlog (reports 0) while closed; here the gate is only an
    // airtime-eligibility valve, so the enclosing compound queue must still see the frames
    // buffered behind it for correct queue-length and capacity accounting.
    virtual int getNumPackets() const override;
    virtual b getTotalLength() const override;
    virtual Packet *getPacket(int index) const override;
    virtual bool isEmpty() const override;
    virtual void removePacket(Packet *packet) override;
    virtual void removeAllPackets() override;
    //@}
};

} // namespace ieee80211
} // namespace inet

#endif
