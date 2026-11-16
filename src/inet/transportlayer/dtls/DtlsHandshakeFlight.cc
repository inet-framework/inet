// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsHandshakeFlight.h"

#include <algorithm>
#include <iterator>

#include "inet/common/packet/Packet.h"

namespace inet::dtls {

size_t DtlsHandshakeFlight::add(uint8_t messageType, uint16_t messageSequence, uint32_t messageLength, uint32_t offset, uint64_t epoch, const Ptr<const Chunk>& data, bool establishAfterSend, bool serverFinished)
{
    if (!data || !data->isImmutable() || messageLength > 0xffffff || offset > messageLength ||
        data->getChunkLength().get<b>() % 8 != 0 || data->getChunkLength() > B(messageLength - offset))
        throw cRuntimeError("Invalid DTLS flight fragment");
    fragments.push_back({messageType, messageSequence, messageLength, offset, epoch, data, false, false, false, false, establishAfterSend, serverFinished, 0, false, {}});
    return fragments.size() - 1;
}

void DtlsHandshakeFlight::markReady(size_t index)
{
    fragments.at(index).ready = true;
    for (auto replacement : fragments.at(index).replacements)
        markReady(replacement);
}

bool DtlsHandshakeFlight::refragment(size_t maximumBodyBytes)
{
    if (maximumBodyBytes == 0)
        throw cRuntimeError("DTLS fragment budget must be positive");
    bool changed = false;
    auto count = fragments.size();
    for (size_t i = 0; i < count; ++i) {
        auto original = fragments[i];
        if (original.epoch == 0 || original.superseded || original.acknowledged || original.data->getChunkLength() <= B(maximumBodyBytes))
            continue;
        fragments[i].superseded = true;
        Packet source("DTLS retained fragment", original.data);
        auto size = original.data->getChunkLength().get<B>();
        for (size_t offset = 0; offset < size; offset += maximumBodyBytes) {
            auto length = std::min<size_t>(maximumBodyBytes, size - offset);
            bool last = offset + length == size;
            auto index = add(original.messageType, original.messageSequence, original.messageLength, original.offset + offset,
                             original.epoch, source.peekDataAt(B(offset), B(length)), last && original.establishAfterSend, last && original.serverFinished);
            fragments[index].ready = original.ready;
            fragments[index].transmissions = original.transmissions;
            fragments[index].retransmissionDue = original.transmissions != 0;
            fragments[i].replacements.push_back(index);
        }
        changed = true;
    }
    return changed;
}

bool DtlsHandshakeFlight::reserve(size_t index)
{
    auto& fragment = fragments.at(index);
    if (fragment.superseded || !fragment.ready || fragment.acknowledged || fragment.queued ||
        (fragment.transmissions != 0 && !fragment.retransmissionDue) || !isFirstTransmissionOrdered(index))
        return false;
    fragment.queued = true;
    return true;
}

bool DtlsHandshakeFlight::isFirstTransmissionOrdered(size_t index) const
{
    const auto& fragment = fragments.at(index);
    if (fragment.superseded)
        return false;
    if (fragment.transmissions != 0 || std::none_of(fragments.begin(), fragments.end(), [](const auto& part) { return part.superseded; }))
        return true;
    for (const auto& part : fragments)
        if (!part.superseded && part.messageSequence < fragment.messageSequence && part.transmissions == 0)
            return false;
    return true;
}

void DtlsHandshakeFlight::requestRetransmission()
{
    for (auto& fragment : fragments)
        if (!fragment.superseded && !fragment.acknowledged && fragment.transmissions != 0)
            fragment.retransmissionDue = true;
}

DtlsHandshakeFlight::AckResult DtlsHandshakeFlight::acknowledgeSent(simtime_t time)
{
    AckResult result;
    for (const auto& entry : sentRecords) {
        auto acknowledged = acknowledge(entry.first.first, entry.first.second, time);
        if (acknowledged.matched) {
            result.matched = true;
            if (acknowledged.rtt)
                result.rtt = acknowledged.rtt;
        }
    }
    result.flightComplete = isComplete();
    return result;
}

void DtlsHandshakeFlight::cancelReservation(size_t index)
{
    fragments.at(index).queued = false;
}

void DtlsHandshakeFlight::handedOff(size_t index, uint64_t recordNumber, simtime_t time)
{
    // RFC 9147 Sections 5.8.2 and 7.2: only handed-off records can be acknowledged or timed.
    auto& fragment = fragments.at(index);
    if (fragment.superseded || !fragment.queued || fragment.acknowledged)
        throw cRuntimeError("DTLS flight fragment was not reserved for transmission");
    auto key = std::make_pair(fragment.epoch, recordNumber);
    if (sentRecords.find(key) != sentRecords.end())
        throw cRuntimeError("DTLS flight record number was reused");
    sentRecords.emplace(key, SentRecord{fragment.messageSequence, fragment.offset, uint32_t(fragment.data->getChunkLength().get<B>()), time, fragment.transmissions != 0});
    fragment.queued = false;
    fragment.retransmissionDue = false;
    ++fragment.transmissions;
}

DtlsHandshakeFlight::AckResult DtlsHandshakeFlight::acknowledge(uint64_t epoch, uint64_t recordNumber, simtime_t time)
{
    // RFC 9147 Section 5.8.2 excludes retransmitted messages from RTT samples.
    AckResult result;
    auto it = sentRecords.find(std::make_pair(epoch, recordNumber));
    if (it == sentRecords.end())
        return result;
    auto message = std::make_pair(epoch, it->second.messageSequence);
    auto& ranges = acknowledgedRanges[message];
    auto begin = it->second.offset;
    auto end = begin + it->second.length;
    auto range = ranges.lower_bound(begin);
    if (range != ranges.begin()) {
        auto previous = std::prev(range);
        if (previous->second >= begin)
            range = previous;
    }
    while (range != ranges.end() && range->first <= end) {
        begin = std::min(begin, range->first);
        end = std::max(end, range->second);
        range = ranges.erase(range);
    }
    ranges.emplace(begin, end);
    for (auto& part : fragments) {
        if (part.superseded || part.acknowledged || part.epoch != epoch || part.messageSequence != message.second)
            continue;
        auto covering = ranges.upper_bound(part.offset);
        if (covering != ranges.begin() && std::prev(covering)->second >= part.offset + part.data->getChunkLength().get<B>()) {
            part.acknowledged = true;
            result.matched = true;
        }
    }
    if (!result.matched)
        return result;
    if (measuredMessages.find(message) == measuredMessages.end()) {
        bool complete = true, eligible = true;
        for (const auto& part : fragments) {
            if (!part.superseded && part.epoch == message.first && part.messageSequence == message.second) {
                complete &= part.acknowledged;
                eligible &= part.transmissions == 1;
            }
        }
        if (complete) {
            measuredMessages.insert(message);
            if (eligible) {
                simtime_t lastHandoff = SIMTIME_ZERO;
                for (const auto& sent : sentRecords) {
                    if (sent.first.first == message.first && sent.second.messageSequence == message.second && !sent.second.retransmitted)
                        lastHandoff = std::max(lastHandoff, sent.second.handoffTime);
                }
                if (time >= lastHandoff)
                    result.rtt = time - lastHandoff;
            }
        }
    }
    result.flightComplete = isComplete();
    return result;
}

bool DtlsHandshakeFlight::isComplete() const
{
    if (fragments.empty())
        return false;
    for (const auto& fragment : fragments)
        if (!fragment.superseded && !fragment.acknowledged)
            return false;
    return true;
}

} // namespace inet::dtls
