//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockackreordering/ReceiveBuffer.h"

#include <algorithm>

namespace inet {
namespace ieee80211 {

ReceiveBuffer::ReceiveBuffer(int bufferSize, SequenceNumberCyclic nextExpectedSequenceNumber) :
    bufferSize(bufferSize),
    nextExpectedSequenceNumber(nextExpectedSequenceNumber)
{
}

bool ReceiveBuffer::isComplete(const Fragments& fragments)
{
    FragmentNumber terminalFragmentNumber = -1;
    bool hasContradictoryTerminalMarkers = false;
    std::set<FragmentNumber> fragmentNumbers;
    for (auto fragment : fragments) {
        if (fragment == nullptr)
            continue;
        const auto& header = fragment->peekAtFront<Ieee80211DataHeader>();
        if (!header->getMoreFragments()) {
            if (terminalFragmentNumber == -1)
                terminalFragmentNumber = header->getFragmentNumber();
            else if (terminalFragmentNumber != header->getFragmentNumber())
                hasContradictoryTerminalMarkers = true;
        }
        fragmentNumbers.insert(header->getFragmentNumber());
    }
    if (terminalFragmentNumber < 0 || hasContradictoryTerminalMarkers || fragmentNumbers.size() != (size_t)terminalFragmentNumber + 1)
        return false;
    for (FragmentNumber fragmentNumber = 0; fragmentNumber <= terminalFragmentNumber; fragmentNumber++)
        if (fragmentNumbers.find(fragmentNumber) == fragmentNumbers.end())
            return false;
    return true;
}

void ReceiveBuffer::pruneExpiredFragmentSequences()
{
    for (auto it = expiredFragmentSequences.begin(); it != expiredFragmentSequences.end();) {
        if (SequenceNumberCyclic(*it) < nextExpectedSequenceNumber)
            it = expiredFragmentSequences.erase(it);
        else
            ++it;
    }
}

//
// Upon the receipt of a QoS data frame from the originator for which a Block Ack agreement exists, the recipient
// buffers the MSDU regardless of the value of the Ack Policy subfield within the QoS Control field of the QoS
// data frame, unless the sequence number of the frame is older than the NextExpectedSequenceNumber for that
// Block Ack agreement, in which case the frame is discarded because it is either old or a duplicate.
//
ReceiveBuffer::FrameInsertionResult ReceiveBuffer::insertFrameWithResult(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader)
{
    auto sequenceNumber = dataHeader->getSequenceNumber();
    auto fragmentNumber = dataHeader->getFragmentNumber();
    bool isFragmented = dataHeader->getMoreFragments() || fragmentNumber != 0;
    pruneExpiredFragmentSequences();
    if (isFragmented && expiredFragmentSequences.find(sequenceNumber.get()) != expiredFragmentSequences.end())
        return FrameInsertionResult::REJECTED_EXPIRED;
    // The total number of MPDUs in these MSDUs may not
    // exceed the reorder buffer size in the receiver.
    if (length < bufferSize && nextExpectedSequenceNumber <= sequenceNumber && sequenceNumber < nextExpectedSequenceNumber + bufferSize) {
        auto it = buffer.find(sequenceNumber.get());
        if (it != buffer.end()) {
            auto& fragments = it->second;
            // TODO efficiency
            for (auto fragment : fragments) {
                const auto& fragmentHeader = fragment->peekAtFront<Ieee80211DataHeader>();
                if (fragmentHeader->getSequenceNumber() == sequenceNumber && fragmentHeader->getFragmentNumber() == fragmentNumber)
                    return FrameInsertionResult::REJECTED;
            }
            fragments.push_back(dataPacket);
        }
        else {
            buffer[sequenceNumber.get()].push_back(dataPacket);
            bufferEntries[sequenceNumber.get()] = { simTime(), isFragmented, false };
        }
        // The total number of frames that can be sent depends on the total
        // number of MPDUs in all the outstanding MSDUs.
        length++;
        // Once an entry has received a fragmented MPDU, keep that identity
        // tied to the generation even if a later malformed or
        // unfragmented-shaped header is accepted into the same sequence slot.
        auto& bufferEntry = bufferEntries[sequenceNumber.get()];
        bufferEntry.hasFragmentedIdentity |= isFragmented;
        bufferEntry.receiveLifetimeActive = bufferEntry.hasFragmentedIdentity && !isComplete(buffer[sequenceNumber.get()]);
        return FrameInsertionResult::INSERTED;
    }
    return FrameInsertionResult::REJECTED;
}

void ReceiveBuffer::dropFramesUntil(SequenceNumberCyclic sequenceNumber)
{
    auto it = buffer.begin();
    while (it != buffer.end()) {
        if (SequenceNumberCyclic(it->first) < sequenceNumber) {
            length -= it->second.size();
            for (auto fragment : it->second)
                delete fragment;
            bufferEntries.erase(it->first);
            it = buffer.erase(it);
        }
        else
            it++;
    }
}

void ReceiveBuffer::removeFrame(SequenceNumberCyclic sequenceNumber)
{
    auto it = buffer.find(sequenceNumber.get());
    if (it != buffer.end()) {
        length -= it->second.size();
        bufferEntries.erase(sequenceNumber.get());
        buffer.erase(sequenceNumber.get());
    }
    else
        throw cRuntimeError("Unknown sequence number: %d", sequenceNumber.get());
}

ReceiveBuffer::Fragments ReceiveBuffer::extractFrames()
{
    Fragments frames;
    for (auto& [sequenceNumber, fragments] : buffer)
        frames.insert(frames.end(), fragments.begin(), fragments.end());
    buffer.clear();
    bufferEntries.clear();
    expiredFragmentSequences.clear();
    length = 0;
    return frames;
}

simtime_t ReceiveBuffer::getNextExpirationTime(simtime_t maxReceiveLifetime) const
{
    if (maxReceiveLifetime == SIMTIME_MAX)
        return SIMTIME_MAX;
    simtime_t nextExpirationTime = SIMTIME_MAX;
    for (const auto& [sequenceNumber, entry] : bufferEntries)
        if (entry.receiveLifetimeActive)
            nextExpirationTime = std::min(nextExpirationTime, entry.receptionStartTime + maxReceiveLifetime);
    return nextExpirationTime;
}

ReceiveBuffer::Fragments ReceiveBuffer::removeExpiredFragments(simtime_t currentTime, simtime_t maxReceiveLifetime)
{
    Fragments expiredFragments;
    if (maxReceiveLifetime == SIMTIME_MAX)
        return expiredFragments;
    for (auto it = buffer.begin(); it != buffer.end();) {
        auto metadataIt = bufferEntries.find(it->first);
        ASSERT(metadataIt != bufferEntries.end());
        auto& entry = metadataIt->second;
        if (entry.receiveLifetimeActive && currentTime >= entry.receptionStartTime + maxReceiveLifetime) {
            for (auto fragment : it->second)
                if (fragment != nullptr)
                    expiredFragments.push_back(fragment);
            length -= it->second.size();
            ASSERT(length >= 0);
            expiredFragmentSequences.insert(it->first);
            bufferEntries.erase(metadataIt);
            it = buffer.erase(it);
        }
        else
            ++it;
    }
    pruneExpiredFragmentSequences();
    return expiredFragments;
}

void ReceiveBuffer::setNextExpectedSequenceNumber(SequenceNumberCyclic nextExpectedSequenceNumber)
{
    this->nextExpectedSequenceNumber = nextExpectedSequenceNumber;
    pruneExpiredFragmentSequences();
}

ReceiveBuffer::~ReceiveBuffer()
{
    for (auto fragments : buffer) {
        for (auto fragment : fragments.second)
            delete fragment;
    }
}

} /* namespace ieee80211 */
} /* namespace inet */
