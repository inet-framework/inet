//
// Copyright (C) 2005-2009 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/sctp/SctpQueue.h"

#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {
namespace sctp {

Register_Class(SctpQueue);

SctpQueue::SctpQueue()
{
    assoc = nullptr;
}

SctpQueue::SctpQueue(SctpAssociation *assoc_)
{
    assoc = assoc_;
}

SctpQueue::~SctpQueue()
{
    for (auto& elem : payloadQueue) {
        SctpDataVariables *chunk = elem.second;
        delete chunk->userData;
        delete chunk;
    }
    if (!payloadQueue.empty()) {
        payloadQueue.clear();
    }
}

bool SctpQueue::checkAndInsertChunk(const uint32_t key, SctpDataVariables *chunk)
{
    auto found = payloadQueue.find(key);
    if (found != payloadQueue.end()) {
        return false;
    }
    payloadQueue[key] = chunk;
    return true;
}

uint32_t SctpQueue::getQueueSize() const
{
    return payloadQueue.size();
}

SctpDataVariables *SctpQueue::extractMessage()
{
    if (!payloadQueue.empty()) {
        auto iterator = payloadQueue.begin();
        SctpDataVariables *chunk = iterator->second;
        payloadQueue.erase(iterator);
        return chunk;
    }
    return nullptr;
}

SctpDataVariables *SctpQueue::getAndExtractChunk(const uint32_t tsn)
{
    if (!payloadQueue.empty()) {
        auto iterator = payloadQueue.find(tsn);
        SctpDataVariables *chunk = iterator->second;
        payloadQueue.erase(iterator);
        return chunk;
    }
    return nullptr;
}

void SctpQueue::printQueue() const
{
    EV_DEBUG << "Queue contents:\n";
    for (const auto& elem : payloadQueue) {
        const uint32_t key = elem.first;
        const SctpDataVariables *chunk = elem.second;
        EV_DEBUG << key << ":\t"
                 << "lastDestination=" << chunk->getLastDestination()
                 << " nextDestination=" << chunk->getNextDestination()
                 << " hasBeenAcked=" << chunk->hasBeenAcked
                 << " countsAsOutstanding=" << chunk->countsAsOutstanding
                 << " numberOfRetransmissions=" << chunk->numberOfRetransmissions
                 << endl;
    }
    EV_DEBUG << endl;
}

SctpDataVariables *SctpQueue::getFirstChunk() const
{
    PayloadQueue::const_iterator iterator = payloadQueue.begin();
    SctpDataVariables *chunk = iterator->second;
    return chunk;
}

cMessage *SctpQueue::getMsg(const uint32_t tsn) const
{
    PayloadQueue::const_iterator iterator = payloadQueue.find(tsn);
    if (iterator != payloadQueue.end()) {
        SctpDataVariables *chunk = iterator->second;
        cMessage *msg = check_and_cast<cMessage *>(chunk->userData);
        return msg;
    }
    return nullptr;
}

SctpDataVariables *SctpQueue::getChunk(const uint32_t tsn) const
{
    PayloadQueue::const_iterator iterator = payloadQueue.find(tsn);
    if (iterator != payloadQueue.end()) {
        SctpDataVariables *chunk = iterator->second;
        return chunk;
    }
    return nullptr;
}

SctpDataVariables *SctpQueue::getChunkFast(const uint32_t tsn, bool& firstTime)
{
    if (!firstTime) {
        if (GetChunkFastIterator != payloadQueue.end()) {
            SctpDataVariables *chunk = GetChunkFastIterator->second;
            if (chunk->tsn == tsn) {
                GetChunkFastIterator++;
                return chunk; // Found the right TSN!
            }
        }
        // TSN not found -> needs regular TSN lookup.
    }

    GetChunkFastIterator = payloadQueue.find(tsn);
    if (GetChunkFastIterator != payloadQueue.end()) {
        SctpDataVariables *chunk = GetChunkFastIterator->second;
        GetChunkFastIterator++;
        firstTime = false;
        return chunk;
    }

    return nullptr;
}

void SctpQueue::removeMsg(const uint32_t tsn)
{
    auto iterator = payloadQueue.find(tsn);
    payloadQueue.erase(iterator);
}

bool SctpQueue::deleteMsg(const uint32_t tsn)
{
    auto iterator = payloadQueue.find(tsn);
    if (iterator != payloadQueue.end()) {
        SctpDataVariables *chunk = iterator->second;
        cMessage *msg = check_and_cast<cMessage *>(chunk->userData);
        delete msg;
        payloadQueue.erase(iterator);
        return true;
    }
    return false;
}

int32_t SctpQueue::getNumBytes() const
{
    int32_t qb = 0;
    for (const auto& elem : payloadQueue) {
        qb += (elem.second->len / 8);
    }
    return qb;
}

SctpDataVariables *SctpQueue::dequeueChunkBySSN(const uint16_t ssn)
{
    for (auto iterator = payloadQueue.begin();
         iterator != payloadQueue.end(); iterator++)
    {
        SctpDataVariables *chunk = iterator->second;
        if ((iterator->second->ssn == ssn) &&
            (iterator->second->bbit) &&
            (iterator->second->ebit))
        {
            payloadQueue.erase(iterator);
            return chunk;
        }
    }
    return nullptr;
}

uint16_t SctpQueue::getFirstSsnInQueue(const uint16_t sid)
{
    return payloadQueue.begin()->second->ssn;
}

void SctpQueue::findEarliestOutstandingTsnsForPath(const L3Address& remoteAddress,
        uint32_t& earliestOutstandingTsn,
        uint32_t& rtxEarliestOutstandingTsn) const
{
    bool findEarliestOutstandingTsn = true;
    bool findRTXEarliestOutstandingTsn = true;

    for (const auto& elem : payloadQueue) {
        const SctpDataVariables *chunk = elem.second;
        if (chunk->getLastDestination() == remoteAddress) {
            // ====== Find earliest outstanding TSNs ===========================
            if (chunk->hasBeenAcked == false) {
                if ((findEarliestOutstandingTsn) &&
                    (chunk->numberOfRetransmissions == 0))
                {
                    earliestOutstandingTsn = chunk->tsn;
                }
                findEarliestOutstandingTsn = false;
                if ((findRTXEarliestOutstandingTsn) &&
                    (chunk->numberOfRetransmissions > 0))
                {
                    rtxEarliestOutstandingTsn = chunk->tsn;
                }
                findRTXEarliestOutstandingTsn = false;
            }
        }
    }
}

uint32_t SctpQueue::getSizeOfFirstChunk(const L3Address& remoteAddress)
{
    for (PayloadQueue::const_iterator iterator = payloadQueue.begin();
         iterator != payloadQueue.end(); ++iterator)
    {
        const SctpDataVariables *chunk = iterator->second;
        if (chunk->getNextDestination() == remoteAddress) {
            return chunk->booksize;
        }
    }
    return 0;
}

} // namespace sctp
} // namespace inet

