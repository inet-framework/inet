// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsHandshakeFlight.h"

namespace inet::dtls {

size_t DtlsHandshakeFlight::add(uint8_t messageType, uint16_t messageSequence, uint32_t messageLength, uint32_t offset, uint64_t epoch, const Ptr<const Chunk>& data, bool establishAfterSend, bool serverFinished)
{
    if (!data || !data->isImmutable() || messageLength > 0xffffff || offset > messageLength ||
        data->getChunkLength().get<b>() % 8 != 0 || data->getChunkLength() > B(messageLength - offset))
        throw cRuntimeError("Invalid DTLS flight fragment");
    fragments.push_back({messageType, messageSequence, messageLength, offset, epoch, data, false, false, false, false, establishAfterSend, serverFinished, 0});
    return fragments.size() - 1;
}

bool DtlsHandshakeFlight::reserve(size_t index)
{
    auto& fragment = fragments.at(index);
    if (!fragment.ready || fragment.acknowledged || fragment.queued || (fragment.transmissions != 0 && !fragment.retransmissionDue))
        return false;
    fragment.queued = true;
    return true;
}

void DtlsHandshakeFlight::requestRetransmission()
{
    for (auto& fragment : fragments)
        if (!fragment.acknowledged && fragment.transmissions != 0)
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
    if (!fragment.queued || fragment.acknowledged)
        throw cRuntimeError("DTLS flight fragment was not reserved for transmission");
    auto key = std::make_pair(fragment.epoch, recordNumber);
    if (sentRecords.find(key) != sentRecords.end())
        throw cRuntimeError("DTLS flight record number was reused");
    sentRecords.emplace(key, SentRecord{index, time, fragment.transmissions != 0});
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
    auto& fragment = fragments.at(it->second.fragmentIndex);
    if (fragment.acknowledged)
        return result;
    result.matched = true;
    fragment.acknowledged = true;
    auto message = std::make_pair(fragment.epoch, fragment.messageSequence);
    if (measuredMessages.find(message) == measuredMessages.end()) {
        bool complete = true, eligible = true;
        for (const auto& part : fragments) {
            if (part.epoch == message.first && part.messageSequence == message.second) {
                complete &= part.acknowledged;
                eligible &= part.transmissions == 1;
            }
        }
        if (complete) {
            measuredMessages.insert(message);
            if (eligible) {
                simtime_t lastHandoff = SIMTIME_ZERO;
                for (const auto& sent : sentRecords) {
                    const auto& part = fragments.at(sent.second.fragmentIndex);
                    if (part.epoch == message.first && part.messageSequence == message.second && !sent.second.retransmitted)
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
        if (!fragment.acknowledged)
            return false;
    return true;
}

} // namespace inet::dtls
