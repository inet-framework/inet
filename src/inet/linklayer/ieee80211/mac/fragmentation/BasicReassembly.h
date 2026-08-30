//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BASICREASSEMBLY_H
#define __INET_BASICREASSEMBLY_H

#include <bitset>
#include <map>
#include <tuple>

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IReassembly.h"

namespace inet {
namespace ieee80211 {

class INET_API BasicReassembly : public IReassembly, public cObject
{
  protected:
    static constexpr int NUM_SEQUENCE_NUMBERS = 1 << 12;

    struct ContextKey {
        MacAddress macAddress;
        MacAddress receiverAddress;
        Ieee80211FrameType type;
        Tid tid;
        auto asTuple() const { return std::tie(macAddress, receiverAddress, type, tid); }
        bool operator==(const ContextKey& other) const { return asTuple() == other.asTuple(); }
        bool operator<(const ContextKey& other) const { return asTuple() < other.asTuple(); }
    };
    struct Key {
        MacAddress macAddress;
        MacAddress receiverAddress;
        Ieee80211FrameType type;
        Tid tid;
        SequenceNumber seqNum;
        auto asTuple() const { return std::tie(macAddress, receiverAddress, type, tid, seqNum); }
        ContextKey getContextKey() const { return { macAddress, receiverAddress, type, tid }; }
        bool operator==(const Key& other) const { return asTuple() == other.asTuple(); }
        bool operator<(const Key& other) const { return asTuple() < other.asTuple(); }
    };
    struct Value {
        std::vector<Packet *> fragments;
        uint16_t receivedFragments = 0; // each bit corresponds to a fragment number
        uint16_t allFragments = 0; // bits for all fragments set to one (0..numFragments-1); 0 means unfilled
        simtime_t receptionStartTime;
        // A fragmented fragment 0 collided with this active identity, but the
        // receiver has no metadata that can correlate it to a generation.
        // Keep the identity until receptionStartTime + maxReceiveLifetime so
        // delayed fragments cannot contaminate a later reassembly.
        bool quarantined = false;
    };
    typedef std::map<Key, Value> FragmentsMap;
    // One fixed sequence bitmap is retained for each nonempty
    // (TA, RA, type, TID) context. Empty contexts are erased immediately;
    // purge() is the explicit protocol lifecycle boundary for identities that
    // remain expired. An arbitrary timer would permit late fragments after
    // their IEEE 802.11-2024 10.5 discard window has been forgotten. Active
    // quarantine entries remain in fragmentsMap until that same deadline.
    typedef std::map<ContextKey, std::bitset<NUM_SEQUENCE_NUMBERS>> ExpiredSequenceNumbersMap;
    FragmentsMap fragmentsMap;
    ExpiredSequenceNumbersMap expiredSequenceNumbersMap;
    simtime_t maxReceiveLifetime;

  public:
    BasicReassembly() : maxReceiveLifetime(SimTime(512 * 1024, SIMTIME_US)) {}
    BasicReassembly(simtime_t maxReceiveLifetime) : maxReceiveLifetime(maxReceiveLifetime) {}
    virtual ~BasicReassembly();
    virtual Packet *addFragment(Packet *packet) override;
    virtual void purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber) override;
    virtual simtime_t getNextExpirationTime() const override;
    virtual std::vector<Packet *> removeExpiredFragments(simtime_t currentTime) override;
};

} // namespace ieee80211
} // namespace inet

#endif
