//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/queueing/contract/IPacketCollection.h"
#include "inet/queueing/function/PacketDropperFunction.h"

namespace inet {
namespace ieee80211 {

/**
 * The overflow policy for the ~AirtimeFairnessQueue compound queue. When the shared
 * capacity is exceeded it drops from the station (receiver) with the most queued frames:
 * the tail frame of the longest per-station backlog. This caps a slow station whose
 * slowly-draining backlog would otherwise fill the shared capacity and lock the other
 * stations out (the FQ-CoDel drop-from-longest rule), rather than dropping the just-arrived
 * frame -- which is what lets every station keep getting served under overload.
 */
static Packet *selectPacketFromLongestFlow(queueing::IPacketCollection *collection)
{
    int numPackets = collection->getNumPackets();
    std::map<MacAddress, int> counts;
    std::map<MacAddress, Packet *> lastPacket; // tail frame of each receiver, in collection order
    for (int i = 0; i < numPackets; i++) {
        auto packet = collection->getPacket(i);
        MacAddress receiver = packet->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress();
        counts[receiver]++;
        lastPacket[receiver] = packet;
    }
    MacAddress longest;
    int most = 0;
    for (auto& element : counts)
        if (element.second > most) {
            most = element.second;
            longest = element.first;
        }
    return most > 0 ? lastPacket[longest] : nullptr;
}

Register_Packet_Dropper_Function(Ieee80211LongestFlowDropper, selectPacketFromLongestFlow);

} // namespace ieee80211
} // namespace inet
