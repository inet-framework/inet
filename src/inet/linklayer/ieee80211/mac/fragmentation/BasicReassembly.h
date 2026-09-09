//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BASICREASSEMBLY_H
#define __INET_BASICREASSEMBLY_H

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
    struct Key {
        MacAddress macAddress;
        MacAddress receiverAddress;
        Ieee80211FrameType type;
        Tid tid;
        SequenceNumber seqNum;
        auto asTuple() const { return std::tie(macAddress, receiverAddress, type, tid, seqNum); }
        bool operator==(const Key& other) const { return asTuple() == other.asTuple(); }
        bool operator<(const Key& other) const { return asTuple() < other.asTuple(); }
    };
    struct Value {
        std::vector<Packet *> fragments;
        uint16_t receivedFragments = 0; // each bit corresponds to a fragment number
        uint16_t allFragments = 0; // bits for all fragments set to one (0..numFragments-1); 0 means unfilled
        simtime_t receptionStartTime;
        bool expired = false;
    };
    typedef std::map<Key, Value> FragmentsMap;
    FragmentsMap fragmentsMap;
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
