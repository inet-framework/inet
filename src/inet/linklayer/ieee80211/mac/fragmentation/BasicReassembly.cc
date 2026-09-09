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

BasicReassembly::SequenceSpaceKey BasicReassembly::getSequenceSpaceKey(const ContextKey& contextKey) const
{
    // IEEE Std 802.11-2024, Table 10-5 defines one baseline sequence-number
    // space for frames not covered by a more specific space, and a separate
    // space for individually addressed QoS Data indexed by RA and TID.
    if (contextKey.type == ST_DATA_WITH_QOS && !contextKey.receiverAddress.isMulticast())
        return { contextKey.macAddress, contextKey.receiverAddress, contextKey.tid };
    else
        return { contextKey.macAddress, MacAddress::UNSPECIFIED_ADDRESS, -1 };
}

SequenceNumber BasicReassembly::getRawSequenceNumber(ExtendedSequenceNumber extendedSequenceNumber)
{
    auto rawSequenceNumber = extendedSequenceNumber % NUM_SEQUENCE_NUMBERS;
    if (rawSequenceNumber < 0)
        rawSequenceNumber += NUM_SEQUENCE_NUMBERS;
    return static_cast<SequenceNumber>(rawSequenceNumber);
}

BasicReassembly::SequenceObservation BasicReassembly::observeSequenceNumber(const SequenceSpaceKey& sequenceSpaceKey, SequenceNumber sequenceNumber)
{
    auto sequenceSpaceIt = sequenceSpacesMap.find(sequenceSpaceKey);
    if (sequenceSpaceIt == sequenceSpacesMap.end()) {
        sequenceSpacesMap.emplace(sequenceSpaceKey, SequenceSpaceValue{ sequenceNumber });
        return { sequenceNumber, false };
    }

    auto& highWatermark = sequenceSpaceIt->second.highWatermark;
    auto highWaterSequenceNumber = getRawSequenceNumber(highWatermark);
    auto highWaterCyclic = SequenceNumberCyclic(highWaterSequenceNumber);
    auto sequenceCyclic = SequenceNumberCyclic(sequenceNumber);

    if (sequenceCyclic == highWaterCyclic)
        return { highWatermark, false };

    // SequenceNumberCyclic uses the IEEE 802.11 half-space ordering: a
    // sequence number is ahead when its forward distance is in 1..2047.
    if (highWaterCyclic < sequenceCyclic) {
        auto forwardDistance = (sequenceNumber - highWaterSequenceNumber + NUM_SEQUENCE_NUMBERS) % NUM_SEQUENCE_NUMBERS;
        ASSERT(forwardDistance > 0 && forwardDistance < NUM_SEQUENCE_NUMBERS / 2);
        highWatermark += forwardDistance;
        return { highWatermark, false };
    }
    else if (sequenceCyclic < highWaterCyclic) {
        auto backwardDistance = (highWaterSequenceNumber - sequenceNumber + NUM_SEQUENCE_NUMBERS) % NUM_SEQUENCE_NUMBERS;
        ASSERT(backwardDistance > 0 && backwardDistance < NUM_SEQUENCE_NUMBERS / 2);
        return { highWatermark - backwardDistance, false };
    }

    // Exactly half a sequence-number space has no ordering under the cyclic
    // comparison. A fragmented MPDU must not be assigned a generation on
    // guesswork. An unfragmented MPDU is safe to accept because it is never
    // combined with buffered fragments; leave the high-water unchanged.
    return { highWatermark, true };
}

void BasicReassembly::pruneExpiredSequenceNumbers(const SequenceSpaceKey& sequenceSpaceKey)
{
    auto sequenceSpaceIt = sequenceSpacesMap.find(sequenceSpaceKey);
    if (sequenceSpaceIt == sequenceSpacesMap.end())
        return;
    auto oldestRetainedGeneration = sequenceSpaceIt->second.highWatermark - NUM_SEQUENCE_NUMBERS;
    for (auto contextIt = expiredSequenceNumbersMap.begin(); contextIt != expiredSequenceNumbersMap.end();) {
        if (!(getSequenceSpaceKey(contextIt->first) == sequenceSpaceKey)) {
            ++contextIt;
            continue;
        }
        auto& expiredSequenceNumbers = contextIt->second;
        for (auto sequenceIt = expiredSequenceNumbers.begin(); sequenceIt != expiredSequenceNumbers.end();) {
            if (*sequenceIt < oldestRetainedGeneration)
                sequenceIt = expiredSequenceNumbers.erase(sequenceIt);
            else
                ++sequenceIt;
        }
        if (expiredSequenceNumbers.empty())
            contextIt = expiredSequenceNumbersMap.erase(contextIt);
        else
            ++contextIt;
    }
}

Packet *BasicReassembly::addFragment(Packet *packet)
{
    const auto& header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    ContextKey contextKey;
    contextKey.macAddress = header->getTransmitterAddress();
    contextKey.receiverAddress = header->getReceiverAddress();
    contextKey.type = header->getType();
    contextKey.tid = -1;
    if (header->getType() == ST_DATA_WITH_QOS)
        if (const Ptr<const Ieee80211DataHeader>& qosDataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
            contextKey.tid = qosDataHeader->getTid();
    short fragNum = header->getFragmentNumber();
    ASSERT(fragNum >= 0 && fragNum < MAX_NUM_FRAGMENTS);
    bool isFragmented = header->getMoreFragments() || fragNum != 0;
    auto sequenceSpaceKey = getSequenceSpaceKey(contextKey);
    // This observed-epoch policy can separate only wraps for which sequence
    // progress was received. A completely unseen wrap is indistinguishable
    // from the same generation with the MAC metadata available here, so an
    // equal raw sequence remains in the current observed epoch to preserve
    // valid out-of-order completion.
    auto sequenceObservation = observeSequenceNumber(sequenceSpaceKey, header->getSequenceNumber().get());

    if (sequenceObservation.ambiguous) {
        if (isFragmented) {
            delete packet;
            return nullptr;
        }
        return packet;
    }

    Key key;
    key.macAddress = contextKey.macAddress;
    key.receiverAddress = contextKey.receiverAddress;
    key.type = contextKey.type;
    key.tid = contextKey.tid;
    key.extendedSequenceNumber = sequenceObservation.extendedSequenceNumber;

    auto it = fragmentsMap.find(key);

    auto expiredIt = expiredSequenceNumbersMap.find(contextKey);
    if (isFragmented && expiredIt != expiredSequenceNumbersMap.end() &&
            expiredIt->second.find(key.extendedSequenceNumber) != expiredIt->second.end()) {
        // A tombstone is scoped to the observed extended generation. Keep it
        // even when a non-Retry fragment 0 arrives: without an observed
        // sequence-space advance that fragment is not distinguishable from a
        // late fragment of the expired MPDU.
        delete packet;
        return nullptr;
    }

    // An unfragmented MPDU is never assembled with a fragmented MPDU. A new
    // non-Retry frame retires an active same-generation reassembly, while a
    // Retry leaves it available for its original fragments.
    if (!isFragmented) {
        if (it != fragmentsMap.end() && !header->getRetry()) {
            for (auto fragment : it->second.fragments)
                if (fragment != nullptr)
                    delete fragment;
            fragmentsMap.erase(it);
        }
        pruneExpiredSequenceNumbers(sequenceSpaceKey);
        return packet;
    }

    // A same-raw active or expired entry in another observed epoch makes all
    // fragments of the new epoch ambiguous, including fragment 0. Quarantine
    // the new extended identity rather than allowing a delayed old fragment
    // to seed a hybrid frame. IEEE Std 802.11-2024, 9.2.4.1.6, 10.3.2.14.2,
    // and 10.5
    // define the modulo-4096 sequence identity, fragment identity, and
    // receive-lifetime discard boundary; the following recovery policy keeps
    // the ambiguous generation rejected while allowing a later observed
    // generation to make progress. A fragmented non-Retry fragment 0 is the
    // only marker that can retire older same-raw tombstones, and only when no
    // active same-raw generation remains. A tombstone for the incoming
    // generation is never cleared by this path.
    bool hasOtherGeneration = false;
    for (const auto& entry : fragmentsMap) {
        const auto& otherKey = entry.first;
        if (otherKey.getContextKey() == contextKey &&
                getRawSequenceNumber(otherKey.extendedSequenceNumber) == header->getSequenceNumber().get() &&
                otherKey.extendedSequenceNumber != key.extendedSequenceNumber) {
            hasOtherGeneration = true;
            break;
        }
    }
    bool isNewGenerationMarker = fragNum == 0 && !header->getRetry();
    if (!hasOtherGeneration && isNewGenerationMarker && expiredIt != expiredSequenceNumbersMap.end()) {
        for (auto sequenceIt = expiredIt->second.begin(); sequenceIt != expiredIt->second.end();) {
            if (*sequenceIt < key.extendedSequenceNumber &&
                    getRawSequenceNumber(*sequenceIt) == header->getSequenceNumber().get())
                sequenceIt = expiredIt->second.erase(sequenceIt);
            else
                ++sequenceIt;
        }
        if (expiredIt->second.empty())
            expiredSequenceNumbersMap.erase(expiredIt);
        expiredIt = expiredSequenceNumbersMap.find(contextKey);
    }
    if (!hasOtherGeneration && expiredIt != expiredSequenceNumbersMap.end()) {
        for (auto extendedSequenceNumber : expiredIt->second) {
            if (extendedSequenceNumber != key.extendedSequenceNumber &&
                    getRawSequenceNumber(extendedSequenceNumber) == header->getSequenceNumber().get()) {
                hasOtherGeneration = true;
                break;
            }
        }
    }
    if (hasOtherGeneration) {
        expiredSequenceNumbersMap[contextKey].insert(key.extendedSequenceNumber);
        pruneExpiredSequenceNumbers(sequenceSpaceKey);
        delete packet;
        return nullptr;
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
    if (!header->getMoreFragments()) {
        if (value.terminalFragmentNumber == -1)
            value.terminalFragmentNumber = fragNum;
        else if (value.terminalFragmentNumber != fragNum)
            value.hasContradictoryTerminalFragmentNumbers = true;
    }
    if (!value.fragments[fragNum]) {
        value.receivedFragments |= fragmentBit;
        if (!header->getMoreFragments() && value.allFragments == 0)
            value.allFragments = (fragmentBit << 1) - 1;
        value.fragments[fragNum] = packet;
    }
    else
        delete packet;

//    MacAddress txAddress = header->getTransmitterAddress();

    // if all fragments arrived, return assembled frame
    if (!value.hasContradictoryTerminalFragmentNumbers && value.allFragments != 0 && value.allFragments == value.receivedFragments) {
        Defragmentation defragmentation;
        value.fragments.erase(std::remove(value.fragments.begin(), value.fragments.end(), nullptr), value.fragments.end());
        auto defragmentedFrame = defragmentation.defragmentFrames(&value.fragments);
        // We need to restore some data from the carrying frame's header like TX address
        // TODO Maybe we need to restore the fromDs, toDs fields as well when traveling through multiple APs
        // TODO Are there any other fields that we need to restore?
        for (auto fragment : value.fragments)
            delete fragment;
        fragmentsMap.erase(key);
        pruneExpiredSequenceNumbers(sequenceSpaceKey);
        return defragmentedFrame;
    }
    else {
        pruneExpiredSequenceNumbers(sequenceSpaceKey);
        return nullptr;
    }
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
            auto contextKey = entry.first.getContextKey();
            auto sequenceSpaceKey = getSequenceSpaceKey(contextKey);
            expiredSequenceNumbersMap[contextKey].insert(entry.first.extendedSequenceNumber);
            it = fragmentsMap.erase(it);
            pruneExpiredSequenceNumbers(sequenceSpaceKey);
        }
        else
            ++it;
    }
    return expiredFragments;
}

std::vector<Packet *> BasicReassembly::purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber)
{
    std::vector<Packet *> purgedFragments;
    auto isInSequenceRange = [startSeqNumber, endSeqNumber](int sequenceNumber) {
        return startSeqNumber <= endSeqNumber ?
                sequenceNumber >= startSeqNumber && sequenceNumber <= endSeqNumber :
                sequenceNumber >= startSeqNumber || sequenceNumber <= endSeqNumber;
    };
    for (auto it = fragmentsMap.begin(); it != fragmentsMap.end();) {
        auto sequenceNumber = getRawSequenceNumber(it->first.extendedSequenceNumber);
        if (it->first.macAddress == address && it->first.tid == tid && isInSequenceRange(sequenceNumber)) {
            for (auto fragment : it->second.fragments)
                if (fragment != nullptr)
                    purgedFragments.push_back(fragment);
            it = fragmentsMap.erase(it);
        }
        else
            it++;
    }

    for (auto it = expiredSequenceNumbersMap.begin(); it != expiredSequenceNumbersMap.end();) {
        if (it->first.macAddress == address && it->first.tid == tid) {
            for (auto sequenceIt = it->second.begin(); sequenceIt != it->second.end();) {
                if (isInSequenceRange(getRawSequenceNumber(*sequenceIt)))
                    sequenceIt = it->second.erase(sequenceIt);
                else
                    ++sequenceIt;
            }
            if (it->second.empty())
                it = expiredSequenceNumbersMap.erase(it);
            else
                ++it;
        }
        else
            ++it;
    }
    return purgedFragments;
}

BasicReassembly::~BasicReassembly()
{
    for (auto it = fragmentsMap.begin(); it != fragmentsMap.end(); ++it)
        for (auto fragment : it->second.fragments)
            delete fragment;
}

} // namespace ieee80211
} // namespace inet
