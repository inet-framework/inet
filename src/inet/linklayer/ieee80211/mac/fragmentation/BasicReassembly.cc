//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include <algorithm>

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Defragmentation.h"

namespace inet {
namespace ieee80211 {

Register_Class(BasicReassembly);

/*
 * FIXME: this function needs a serious review
 */
Packet *BasicReassembly::addFragment(Packet *packet)
{
    const auto& header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    // Frame is not fragmented
    if (!header->getMoreFragments() && header->getFragmentNumber() == 0)
        return packet;
    // FIXME: temporary fix for mgmt frames
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(header))
        return packet;
    // find entry for this frame
    Key key;
    key.macAddress = header->getTransmitterAddress();
    key.tid = -1;
    if (header->getType() == ST_DATA_WITH_QOS)
        if (const Ptr<const Ieee80211DataHeader>& qosDataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
            key.tid = qosDataHeader->getTid();
    key.seqNum = header->getSequenceNumber().get();
    short fragNum = header->getFragmentNumber();
    ASSERT(fragNum >= 0 && fragNum < MAX_NUM_FRAGMENTS);
    auto& value = fragmentsMap[key];
    value.fragments.resize(16);

    // update entry
    uint16_t fragmentBit = 1 << fragNum;
    value.receivedFragments |= fragmentBit;
    if (!header->getMoreFragments())
        value.allFragments = (fragmentBit << 1) - 1;
    if (!value.fragments[fragNum])
        value.fragments[fragNum] = packet;
    else
        delete packet;

    //MacAddress txAddress = header->getTransmitterAddress();

    // if all fragments arrived, return assembled frame
    if (value.allFragments != 0 && value.allFragments == value.receivedFragments) {
        Defragmentation defragmentation;
        value.fragments.erase(std::remove(value.fragments.begin(), value.fragments.end(), nullptr), value.fragments.end());
        auto defragmentedFrame = defragmentation.defragmentFrames(&value.fragments);
        // We need to restore some data from the carrying frame's header like TX address
        // TODO: Maybe we need to restore the fromDs, toDs fields as well when traveling through multiple APs
        // TODO: Are there any other fields that we need to restore?
        for (auto fragment : value.fragments)
            delete fragment;
        fragmentsMap.erase(key);
        return defragmentedFrame;
    }
    else
        return nullptr;
}

void BasicReassembly::purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber)
{
    Key key;
    key.macAddress = address;
    key.tid = tid;
    key.seqNum = startSeqNumber;
    auto itStart = fragmentsMap.lower_bound(key);
    key.seqNum = endSeqNumber;
    auto itEnd = fragmentsMap.upper_bound(key);

    if (endSeqNumber < startSeqNumber) {
        for (auto it = itStart; it != fragmentsMap.end(); ) {
            for (auto fragment : it->second.fragments)
                delete fragment;
            it = fragmentsMap.erase(it);
        }
        itStart = fragmentsMap.begin();
    }
    for (auto it = itStart; it != itEnd; ) {
        for (auto fragment : it->second.fragments)
            delete fragment;
        it = fragmentsMap.erase(it);
    }
}

BasicReassembly::~BasicReassembly()
{
    for (auto it = fragmentsMap.begin(); it != fragmentsMap.end(); ++it)
        for (auto fragment : it->second.fragments)
            delete fragment;
}

} // namespace ieee80211
} // namespace inet

