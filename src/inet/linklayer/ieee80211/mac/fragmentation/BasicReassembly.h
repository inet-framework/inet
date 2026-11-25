//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BASICREASSEMBLY_H
#define __INET_BASICREASSEMBLY_H

#include <tuple>

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IReassembly.h"

namespace inet {
namespace ieee80211 {

class INET_API BasicReassembly : public IReassembly, public cObject
{
  protected:
    struct Key {
        MacAddress transmitterAddress;
        Tid tid;
        SequenceNumber seqNum;
        MacAddress groupAddress;

        bool operator==(const Key& other) const {
            return std::tie(transmitterAddress, tid, seqNum, groupAddress) ==
                   std::tie(other.transmitterAddress, other.tid, other.seqNum, other.groupAddress);
        }
        bool operator<(const Key& other) const {
            return std::tie(transmitterAddress, tid, seqNum, groupAddress) <
                   std::tie(other.transmitterAddress, other.tid, other.seqNum, other.groupAddress);
        }
    };
    struct Value {
        std::vector<Packet *> fragments;
        uint16_t receivedFragments = 0; // each bit corresponds to a fragment number
        uint16_t allFragments = 0; // bits for all fragments set to one (0..numFragments-1); 0 means unfilled
    };
    typedef std::map<Key, Value> FragmentsMap;
    FragmentsMap fragmentsMap;

  public:
    virtual ~BasicReassembly();
    virtual Packet *addFragment(Packet *packet) override;
    virtual void purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber) override;
};

} // namespace ieee80211
} // namespace inet

#endif

