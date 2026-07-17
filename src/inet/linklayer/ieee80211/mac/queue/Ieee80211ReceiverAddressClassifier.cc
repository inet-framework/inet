//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/queueing/contract/IPacketClassifierFunction.h"

namespace inet {
namespace ieee80211 {

/**
 * A packet classifier function that classifies IEEE 802.11 frames by receiver (destination
 * station) MAC address, assigning a dense class index in first-seen order. Used with
 * ~DynamicClassifier to give one per-receiver branch (sub-queue + airtime gate) in
 * ~AirtimeFairnessQueue.
 */
class INET_API Ieee80211ReceiverAddressClassifier : public cObject, public virtual queueing::IPacketClassifierFunction
{
  protected:
    mutable std::map<MacAddress, int> addressToIndex; // receiver MAC -> dense class index, first-seen order

  public:
    virtual int classifyPacket(Packet *packet) const override
    {
        MacAddress address = packet->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress();
        auto it = addressToIndex.find(address);
        if (it != addressToIndex.end())
            return it->second;
        int index = (int)addressToIndex.size();
        addressToIndex[address] = index;
        return index;
    }
};

Register_Class(Ieee80211ReceiverAddressClassifier);

} // namespace ieee80211
} // namespace inet
