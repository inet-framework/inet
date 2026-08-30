//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"

#include <algorithm>

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Defragmentation.h"

namespace inet {
namespace ieee80211 {

Register_Class(BasicReassembly);

/*
 * FIXME this function needs a serious review
 */
Packet *BasicReassembly::addFragment(Packet *packet)
{
    const auto& header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    // Frame is not fragmented
    if (!header->getMoreFragments() && header->getFragmentNumber() == 0)
        return packet;
    // find entry for this frame
    Key key;
    key.macAddress = header->getTransmitterAddress();
    key.receiverAddress = header->getReceiverAddress();
    key.type = header->getType();
    key.tid = -1;
    if (header->getType() == ST_DATA_WITH_QOS)
        if (const Ptr<const Ieee80211DataHeader>& qosDataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
            key.tid = qosDataHeader->getTid();
    key.seqNum = header->getSequenceNumber().get();
    short fragNum = header->getFragmentNumber();
    ASSERT(fragNum >= 0 && fragNum < MAX_NUM_FRAGMENTS);
    auto it = fragmentsMap.find(key);
    if (it != fragmentsMap.end() && it->second.expired) {
        // A non-Retry fragment 0 can be a new MMPDU after sequence-number
        // reuse. All other fragments of the expired MMPDU are discarded.
        if (fragNum == 0 && !header->getRetry()) {
            fragmentsMap.erase(it);
            it = fragmentsMap.end();
        }
        else {
            delete packet;
            return nullptr;
        }
    }
    if (it == fragmentsMap.end()) {
        Value value;
        value.receptionStartTime = simTime();
        it = fragmentsMap.emplace(key, value).first;
    }
    auto& value = it->second;
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

//    MacAddress txAddress = header->getTransmitterAddress();

    // if all fragments arrived, return assembled frame
    if (value.allFragments != 0 && value.allFragments == value.receivedFragments) {
        Defragmentation defragmentation;
        value.fragments.erase(std::remove(value.fragments.begin(), value.fragments.end(), nullptr), value.fragments.end());
        auto defragmentedFrame = defragmentation.defragmentFrames(&value.fragments);
        // We need to restore some data from the carrying frame's header like TX address
        // TODO Maybe we need to restore the fromDs, toDs fields as well when traveling through multiple APs
        // TODO Are there any other fields that we need to restore?
        for (auto fragment : value.fragments)
            delete fragment;
        fragmentsMap.erase(key);
        return defragmentedFrame;
    }
    else
        return nullptr;
}

simtime_t BasicReassembly::getNextExpirationTime() const
{
    simtime_t nextExpirationTime = SIMTIME_MAX;
    for (const auto& entry : fragmentsMap)
        if (!entry.second.expired)
            nextExpirationTime = std::min(nextExpirationTime, entry.second.receptionStartTime + maxReceiveLifetime);
    return nextExpirationTime;
}

std::vector<Packet *> BasicReassembly::removeExpiredFragments(simtime_t currentTime)
{
    std::vector<Packet *> expiredFragments;
    for (auto& entry : fragmentsMap) {
        auto& value = entry.second;
        if (!value.expired && currentTime >= value.receptionStartTime + maxReceiveLifetime) {
            for (auto fragment : value.fragments)
                if (fragment != nullptr)
                    expiredFragments.push_back(fragment);
            value.fragments.clear();
            value.receivedFragments = 0;
            value.allFragments = 0;
            value.expired = true;
        }
    }
    return expiredFragments;
}

void BasicReassembly::purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber)
{
    for (auto it = fragmentsMap.begin(); it != fragmentsMap.end();) {
        auto sequenceNumber = it->first.seqNum;
        bool isInSequenceRange = startSeqNumber <= endSeqNumber ?
                sequenceNumber >= startSeqNumber && sequenceNumber <= endSeqNumber :
                sequenceNumber >= startSeqNumber || sequenceNumber <= endSeqNumber;
        if (it->first.macAddress == address && it->first.tid == tid && isInSequenceRange) {
            for (auto fragment : it->second.fragments)
                delete fragment;
            it = fragmentsMap.erase(it);
        }
        else
            it++;
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
