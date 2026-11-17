// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsHandshakeReassembly.h"

#include <algorithm>

namespace inet::dtls {

void DtlsHandshakeReassembly::discard(uint16_t sequence)
{
    auto it = entries.find(sequence);
    if (it != entries.end()) {
        reservedBytes -= std::max(size_t(1), size_t(it->second.totalLength));
        entries.erase(it);
    }
}

DtlsHandshakeReassembly::Result DtlsHandshakeReassembly::insert(uint8_t type, uint16_t sequence, uint32_t totalLength, uint32_t offset, const std::vector<uint8_t>& fragment)
{
    // RFC 9147 Section 5.5 permits overlapping fragments; the reservation cap is a model resource policy.
    if (totalLength > 0xffffff || offset > totalLength || fragment.size() > totalLength - offset)
        return Result::OUT_OF_RANGE;
    auto it = entries.find(sequence);
    if (it != entries.end() && (it->second.type != type || it->second.totalLength != totalLength))
        return Result::CONFLICT;
    if (fragment.empty())
        return Result::EMPTY;
    if (it == entries.end()) {
        auto reservation = std::max(size_t(1), size_t(totalLength));
        if (reservation > limit || reservedBytes > limit - reservation)
            return Result::LIMIT;
        Entry entry{type, totalLength, std::vector<uint8_t>(totalLength), std::vector<bool>(totalLength), 0};
        it = entries.emplace(sequence, std::move(entry)).first;
        reservedBytes += reservation;
    }
    auto& entry = it->second;
    // Check all overlapping bytes before making any change.
    for (size_t i = 0; i < fragment.size(); ++i)
        if (entry.present[offset + i] && entry.bytes[offset + i] != fragment[i])
            return Result::CONFLICT;
    for (size_t i = 0; i < fragment.size(); ++i) {
        auto index = offset + i;
        if (!entry.present[index]) {
            entry.present[index] = true;
            entry.bytes[index] = fragment[i];
            ++entry.received;
        }
    }
    return entry.received == entry.totalLength ? Result::COMPLETE : Result::INCOMPLETE;
}

bool DtlsHandshakeReassembly::isComplete(uint16_t sequence) const
{
    auto it = entries.find(sequence);
    return it != entries.end() && it->second.received == it->second.totalLength;
}

DtlsHandshakeReassembly::Result DtlsHandshakeReassembly::insertEmptyMessage(uint8_t type, uint16_t sequence)
{
    auto it = entries.find(sequence);
    if (it != entries.end())
        return it->second.type == type && it->second.totalLength == 0 ? Result::COMPLETE : Result::CONFLICT;
    if (reservedBytes >= limit)
        return Result::LIMIT;
    entries.emplace(sequence, Entry{type, 0, {}, {}, 0});
    ++reservedBytes;
    return Result::COMPLETE;
}

size_t DtlsHandshakeReassembly::firstMissing(uint16_t sequence) const
{
    auto it = entries.find(sequence);
    if (it == entries.end())
        return 0;
    auto missing = std::find(it->second.present.begin(), it->second.present.end(), false);
    return missing - it->second.present.begin();
}

std::vector<uint8_t> DtlsHandshakeReassembly::getContiguousPrefix(uint16_t sequence) const
{
    auto it = entries.find(sequence);
    if (it == entries.end())
        return {};
    return {it->second.bytes.begin(), it->second.bytes.begin() + firstMissing(sequence)};
}

DtlsHandshakeReassembly::Message DtlsHandshakeReassembly::take(uint16_t sequence)
{
    auto it = entries.find(sequence);
    if (it == entries.end() || it->second.received != it->second.totalLength)
        throw cRuntimeError("DTLS handshake message is not complete");
    Message message{it->second.type, std::move(it->second.bytes)};
    reservedBytes -= std::max(size_t(1), size_t(it->second.totalLength));
    entries.erase(it);
    return message;
}

void DtlsHandshakeReassembly::clear()
{
    entries.clear();
    reservedBytes = 0;
}

} // namespace inet::dtls
