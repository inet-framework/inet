//
// Copyright (C) 2005-2009 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpQueue.h"

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
    for (auto & elem : payloadQueue)
    {
        SctpDataVariables *chunk = elem.second;
        delete chunk->userData;
    }
    if (!payloadQueue.empty()) {
        payloadQueue.clear();
    }
}

bool SctpQueue::checkAndInsertChunk(const uint32 key, SctpDataVariables *chunk)
{
    auto found = payloadQueue.find(key);
    if (found != payloadQueue.end()) {
        return false;
    }
    payloadQueue[key] = chunk;
    return true;
}

uint32 SctpQueue::getQueueSize() const
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

SctpDataVariables *SctpQueue::getAndExtractChunk(const uint32 tsn)
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
    for (const auto & elem : payloadQueue)
    {
        const uint32 key = elem.first;
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

cMessage *SctpQueue::getMsg(const uint32 tsn) const
{
    PayloadQueue::const_iterator iterator = payloadQueue.find(tsn);
    if (iterator != payloadQueue.end()) {
        SctpDataVariables *chunk = iterator->second;
        cMessage *msg = check_and_cast<cMessage *>(chunk->userData);
        return msg;
    }
    return nullptr;
}

SctpDataVariables *SctpQueue::getChunk(const uint32 tsn) const
{
    PayloadQueue::const_iterator iterator = payloadQueue.find(tsn);
    if (iterator != payloadQueue.end()) {
        SctpDataVariables *chunk = iterator->second;
        return chunk;
    }
    return nullptr;
}

SctpDataVariables *SctpQueue::getChunkFast(const uint32 tsn, bool& firstTime)
{
    if (!firstTime) {
        if (GetChunkFastIterator != payloadQueue.end()) {
            SctpDataVariables *chunk = GetChunkFastIterator->second;
            if (chunk->tsn == tsn) {
                GetChunkFastIterator++;
                return chunk;    // Found the right TSN!
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

void SctpQueue::removeMsg(const uint32 tsn)
{
    auto iterator = payloadQueue.find(tsn);
    payloadQueue.erase(iterator);
}

bool SctpQueue::deleteMsg(const uint32 tsn)
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

int32 SctpQueue::getNumBytes() const
{
    int32 qb = 0;
    for (const auto & elem : payloadQueue)
    {
        qb += (elem.second->len / 8);
    }
    return qb;
}

SctpDataVariables *SctpQueue::dequeueChunkBySSN(const uint16 ssn)
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

uint16 SctpQueue::getFirstSsnInQueue(const uint16 sid)
{
    return payloadQueue.begin()->second->ssn;
}

void SctpQueue::findEarliestOutstandingTsnsForPath(const L3Address& remoteAddress,
        uint32& earliestOutstandingTsn,
        uint32& rtxEarliestOutstandingTsn) const
{
    bool findEarliestOutstandingTsn = true;
    bool findRTXEarliestOutstandingTsn = true;

    for (const auto & elem : payloadQueue)
    {
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

uint32 SctpQueue::getSizeOfFirstChunk(const L3Address& remoteAddress)
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

