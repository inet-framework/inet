//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BASICREASSEMBLY_H
#define __INET_BASICREASSEMBLY_H

#include <map>
#include <set>
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
    using ExtendedSequenceNumber = int64_t;

    struct ContextKey {
        MacAddress macAddress;
        MacAddress receiverAddress;
        Ieee80211FrameType type;
        Tid tid;
        auto asTuple() const { return std::tie(macAddress, receiverAddress, type, tid); }
        bool operator==(const ContextKey& other) const { return asTuple() == other.asTuple(); }
        bool operator<(const ContextKey& other) const { return asTuple() < other.asTuple(); }
    };
    struct SequenceSpaceKey {
        MacAddress transmitterAddress;
        MacAddress receiverAddress;
        Tid tid;
        auto asTuple() const { return std::tie(transmitterAddress, receiverAddress, tid); }
        bool operator==(const SequenceSpaceKey& other) const { return asTuple() == other.asTuple(); }
        bool operator<(const SequenceSpaceKey& other) const { return asTuple() < other.asTuple(); }
    };
    struct SequenceSpaceValue {
        ExtendedSequenceNumber highWatermark;
    };
    struct Key {
        MacAddress macAddress;
        MacAddress receiverAddress;
        Ieee80211FrameType type;
        Tid tid;
        ExtendedSequenceNumber extendedSequenceNumber;
        auto asTuple() const { return std::tie(macAddress, receiverAddress, type, tid, extendedSequenceNumber); }
        ContextKey getContextKey() const { return { macAddress, receiverAddress, type, tid }; }
        bool operator==(const Key& other) const { return asTuple() == other.asTuple(); }
        bool operator<(const Key& other) const { return asTuple() < other.asTuple(); }
    };
    struct Value {
        std::vector<Packet *> fragments;
        uint16_t receivedFragments = 0; // each bit corresponds to a fragment number
        uint16_t allFragments = 0; // bits for all fragments set to one (0..numFragments-1); 0 means unfilled
        int terminalFragmentNumber = -1;
        bool hasContradictoryTerminalFragmentNumbers = false;
        simtime_t receptionStartTime;
    };
    typedef std::map<Key, Value> FragmentsMap;
    typedef std::map<SequenceSpaceKey, SequenceSpaceValue> SequenceSpacesMap;
    using ExpiredSequenceNumbers = std::set<ExtendedSequenceNumber>;
    // Generation-scoped tombstones reject late or ambiguous fragments and
    // retain identities across one raw-sequence reuse. Older entries are
    // pruned as the observed sequence-space high-water advances.
    typedef std::map<ContextKey, ExpiredSequenceNumbers> ExpiredSequenceNumbersMap;
    FragmentsMap fragmentsMap;
    SequenceSpacesMap sequenceSpacesMap;
    ExpiredSequenceNumbersMap expiredSequenceNumbersMap;
    simtime_t maxReceiveLifetime;

    struct SequenceObservation {
        ExtendedSequenceNumber extendedSequenceNumber;
        bool ambiguous = false;
    };

    SequenceSpaceKey getSequenceSpaceKey(const ContextKey& contextKey) const;
    SequenceObservation observeSequenceNumber(const SequenceSpaceKey& sequenceSpaceKey, SequenceNumber sequenceNumber);
    void pruneExpiredSequenceNumbers(const SequenceSpaceKey& sequenceSpaceKey);
    static SequenceNumber getRawSequenceNumber(ExtendedSequenceNumber extendedSequenceNumber);

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
