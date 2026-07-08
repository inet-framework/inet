//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AIRTIMEFAIRNESSQUEUE_H
#define __INET_AIRTIMEFAIRNESSQUEUE_H

#include <deque>
#include <list>
#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/queueing/base/PacketQueueBase.h"

namespace inet {
namespace ieee80211 {

/**
 * A per-station airtime deficit round-robin (DRR) transmit queue for IEEE 802.11,
 * modelled on the Linux `mac80211` airtime-fairness feature. The queue keeps one
 * FIFO sub-queue per receiver (destination MAC address) and hands frames to the MAC
 * so that, over time, every backlogged station gets an equal share of *on-air time*
 * (scaled by an optional per-station weight) rather than an equal share of frames.
 *
 * This fixes the downlink form of the 802.11 rate anomaly: when an access point
 * saturates a mix of fast and slow clients, a plain FIFO lets the slow client's long
 * frames drag the fast clients down to its throughput; airtime fairness holds the slow
 * client to its fair *time* share and lets the fast clients recover.
 *
 * Scheduling (sign-based DRR, as in mac80211):
 *  - Each station has an airtime `deficit`. A station is eligible to transmit while its
 *    deficit is non-negative. When the round-robin cursor reaches an ineligible (backlogged
 *    but negative-deficit) station, the station is topped up by `quantum * weight` and the
 *    cursor advances; this repeats until an eligible station is found.
 *  - The deficit is charged a-posteriori from the *actual* on-air duration of each
 *    transmitted frame (so retries are included), reported by the coordination function
 *    via the `frameTransmittedAirtime` signal (see ~Ieee80211AirtimeInd). Because the MAC
 *    only extracts a new frame after the previous one's transmission completes, the charge
 *    always lands before the next scheduling decision.
 *
 * Drops into the `pendingQueue` slot of a ~Dcaf/~Edcaf via a typename override; the MAC
 * state machine, contention and rate selection are untouched.
 *
 * @see AirtimeFairnessQueue.ned, ~Ieee80211AirtimeInd, ~PendingQueue
 */
class INET_API AirtimeFairnessQueue : public queueing::PacketQueueBase, public cListener
{
  protected:
    // parameters
    int packetCapacity = -1; // total number of packets across all stations, -1 means unlimited
    simtime_t quantum; // base DRR quantum, in airtime units
    double weight = 1.0; // per-station airtime weight (equal for all stations)
    bool fairnessEnabled = true; // when false, degrades to a plain FIFO passthrough

    struct StationState {
        std::deque<Packet *> frames;
        simtime_t deficit = SIMTIME_ZERO;
        double weight = 1.0;
    };

    std::map<MacAddress, StationState> stations; // per-receiver state, kept across drains for late charges
    std::list<MacAddress> activeList; // backlogged stations in round-robin order (each present at most once)
    int numPackets = 0; // total packets across all stations

    simsignal_t frameTransmittedAirtimeSignal = SIMSIGNAL_NULL;

  protected:
    virtual void initialize(int stage) override;

    virtual MacAddress getReceiverAddress(Packet *packet) const;
    virtual StationState& getOrCreateStation(const MacAddress& address);
    virtual void removeStoredPacket(const MacAddress& address, Packet *packet);

  public:
    virtual ~AirtimeFairnessQueue();

    virtual int getMaxNumPackets() const override { return packetCapacity; }
    virtual int getNumPackets() const override { return numPackets; }

    virtual b getMaxTotalLength() const override { return b(-1); }
    virtual b getTotalLength() const override;

    virtual bool isEmpty() const override { return numPackets == 0; }
    virtual Packet *getPacket(int index) const override;
    virtual void removePacket(Packet *packet) override;
    virtual void removeAllPackets() override;

    virtual bool supportsPacketPushing(const cGate *gate) const override { return inputGate == gate; }
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual bool supportsPacketPulling(const cGate *gate) const override { return outputGate == gate; }
    virtual bool canPullSomePacket(const cGate *gate) const override { return !isEmpty(); }
    virtual Packet *canPullPacket(const cGate *gate) const override { return !isEmpty() ? getPacket(0) : nullptr; }
    virtual Packet *pullPacket(const cGate *gate) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *object, cObject *details) override;
};

} // namespace ieee80211
} // namespace inet

#endif
