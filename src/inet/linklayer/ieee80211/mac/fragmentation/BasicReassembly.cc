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
    // Find the entry for this frame. The sequence number is a 12-bit field,
    // so an expired identity can be represented by one bit in its
    // transmitter/receiver/type/TID context instead of retaining a map node
    // (and an empty fragment vector) for every sequence number.
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
    // IEEE Std 802.11-2024, 10.4/10.5 requires fragmented MPDUs to be
    // reconstructed from their fragment numbers; §26.3.2.1 permits level-3
    // dynamic fragments to arrive out of order. At this boundary the receiver
    // sees only the modulo-4096 sequence identity and cannot distinguish a
    // reused fragmented fragment 0, including one marked Retry, from one
    // belonging to the active generation. Quarantine every fragmented
    // fragment-0 collision instead of replacing or merging state, because
    // delayed fragments from the old generation could otherwise complete a
    // hybrid frame. An unfragmented non-Retry fragment 0 is safe to return
    // after retiring conflicting fragmented state because it is never
    // assembled with it.
    bool isUnfragmented = fragNum == 0 && !header->getMoreFragments();
    if (it != fragmentsMap.end()) {
        auto& value = it->second;
        if (value.quarantined) {
            if (isUnfragmented && !header->getRetry()) {
                fragmentsMap.erase(it);
                it = fragmentsMap.end();
            }
            else {
                delete packet;
                return nullptr;
            }
        }
        else if (fragNum == 0) {
            if (isUnfragmented && !header->getRetry()) {
                for (auto fragment : value.fragments)
                    if (fragment != nullptr)
                        delete fragment;
                fragmentsMap.erase(it);
                it = fragmentsMap.end();
            }
            else if (!isUnfragmented) {
                for (auto fragment : value.fragments)
                    if (fragment != nullptr)
                        delete fragment;
                value.fragments.clear();
                value.receivedFragments = 0;
                value.allFragments = 0;
                value.quarantined = true;
                delete packet;
                return nullptr;
            }
        }
    }
    if (it == fragmentsMap.end()) {
        auto contextKey = key.getContextKey();
        auto expiredIt = expiredSequenceNumbersMap.find(contextKey);
        if (expiredIt != expiredSequenceNumbersMap.end() && expiredIt->second.test(key.seqNum)) {
            // A non-Retry fragment 0 can be a new MMPDU after sequence-number
            // reuse. All other fragments of the expired MMPDU are discarded.
            if (fragNum == 0 && !header->getRetry()) {
                expiredIt->second.reset(key.seqNum);
                if (expiredIt->second.none())
                    expiredSequenceNumbersMap.erase(expiredIt);
            }
            else {
                delete packet;
                return nullptr;
            }
        }
    }

    // Frame is not fragmented. Clear a matching expired identity first so a
    // valid, unfragmented sequence-number reuse also retires the tombstone.
    if (!header->getMoreFragments() && fragNum == 0)
        return packet;

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
        nextExpirationTime = std::min(nextExpirationTime, entry.second.receptionStartTime + maxReceiveLifetime);
    return nextExpirationTime;
}

std::vector<Packet *> BasicReassembly::removeExpiredFragments(simtime_t currentTime)
{
    std::vector<Packet *> expiredFragments;
    for (auto it = fragmentsMap.begin(); it != fragmentsMap.end();) {
        auto& entry = *it;
        auto& value = entry.second;
        if (currentTime >= value.receptionStartTime + maxReceiveLifetime) {
            for (auto fragment : value.fragments)
                if (fragment != nullptr)
                    expiredFragments.push_back(fragment);
            expiredSequenceNumbersMap[entry.first.getContextKey()].set(entry.first.seqNum);
            it = fragmentsMap.erase(it);
        }
        else
            ++it;
    }
    return expiredFragments;
}

void BasicReassembly::purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber)
{
    auto isInSequenceRange = [startSeqNumber, endSeqNumber](int sequenceNumber) {
        return startSeqNumber <= endSeqNumber ?
                sequenceNumber >= startSeqNumber && sequenceNumber <= endSeqNumber :
                sequenceNumber >= startSeqNumber || sequenceNumber <= endSeqNumber;
    };
    for (auto it = fragmentsMap.begin(); it != fragmentsMap.end();) {
        auto sequenceNumber = it->first.seqNum;
        if (it->first.macAddress == address && it->first.tid == tid && isInSequenceRange(sequenceNumber)) {
            for (auto fragment : it->second.fragments)
                delete fragment;
            it = fragmentsMap.erase(it);
        }
        else
            it++;
    }

    for (auto it = expiredSequenceNumbersMap.begin(); it != expiredSequenceNumbersMap.end();) {
        if (it->first.macAddress == address && it->first.tid == tid) {
            for (int sequenceNumber = 0; sequenceNumber < NUM_SEQUENCE_NUMBERS; sequenceNumber++)
                if (isInSequenceRange(sequenceNumber))
                    it->second.reset(sequenceNumber);
            if (it->second.none())
                it = expiredSequenceNumbersMap.erase(it);
            else
                ++it;
        }
        else
            ++it;
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
